#include "Python.h"
#include "structmember.h"
#include "dll.h"

#include <string.h>
#include <stdio.h>

/* type-definition & utility-macros */
typedef struct {
    PyObject_HEAD
    int noOfCards;
    struct dealType deal;
} deal_cell;
staticforward PyTypeObject deal_type;

static char* cardrepr = "AKQJT98765432";

/*
static int ones(unsigned int m) {
    int result = 0;
    while(m>0) {
        if(m&1) result++;
        m>>=1;
    }
    return result;
}
*/

static void showdist(unsigned int ca[4][4]) {
    int h, s;
    // printf("SD ");
    for(h=0; h<4; h++) {
        for(s=0; s<4; s++) {
            // printf("%d", ones(ca[h][s]));
            // if(s==3) printf("  ");
            // else printf("-");
        }
    }
    // printf("\n");
}

static void showdist_short(unsigned short int ca[4][4]) {
    int h, s;
    // printf("SDS ");
    for(h=0; h<4; h++) {
        for(s=0; s<4; s++) {
            // printf("%d", ones(ca[h][s]));
            // if(s==3) printf("  ");
            // else printf("-");
        }
    }
    // printf("\n");
}

/* ctor (init) and dtor (dealloc) */
static int
deal_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char* nams[] = {"deal", NULL};
    PyObject* indeal;
    PyObject* seq;
    int i, j, k;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", nams, &indeal))
        return -1;
    deal_cell *the_deal = (deal_cell*)self;

    seq = PySequence_Fast(indeal, "expected deal -> sequence of 4 hands");
    if(!seq)
        return -1;
    if(PySequence_Fast_GET_SIZE(seq) != 4) {
        char buf[80];
        sprintf(buf, "expected exactly 4 hands, not %d",
                PySequence_Fast_GET_SIZE(seq));
        PyErr_SetString(PyExc_ValueError, buf);
        Py_DECREF(seq);
        return -1;
    }

    PyObject** hands = PySequence_Fast_ITEMS(seq);
    PyObject* handseq[4];
    Py_DECREF(seq);
    for(i=0; i<4; i++) {
        PyObject* h = handseq[i] = PySequence_Fast(hands[i], 
                                   "each hand must be a sequence of 4 suits");
        if(h) {
            if (PySequence_Fast_GET_SIZE(h) != 4) {
                char buf[80];
                sprintf(buf, "expected exactly 4 suits, not %d, for hand %d",
                        PySequence_Fast_GET_SIZE(h), i);
                PyErr_SetString(PyExc_ValueError, buf);
                Py_DECREF(h);
                handseq[i] = 0;
            }
        }
        if(!handseq[i]) {
            for(j=0; j<i; j++)
                Py_DECREF(handseq[j]);
            return -1;
        }
    }

    int numcards = -1;
    for(i=0; i<4; i++) {
        PyObject* h = handseq[i];
        int nc = 0;
        for(k=0; k<4; k++) {
            PyObject* suit = PySequence_Fast(PySequence_Fast_GET_ITEM(h, k),
                                              "each suit must be seq of cards");
            if(suit) {
                nc += PySequence_Fast_GET_SIZE(suit);
                Py_DECREF(suit);
            } else {
                for(j=0; j<4; j++)
                    Py_DECREF(handseq[j]);
                return -1;
            }
        }
        if(numcards>=0 && nc != numcards) {
            char buf[80];
            sprintf(buf, "all hands must have same #cards (%d!=%d at %d)",
                    numcards, nc, i);
            PyErr_SetString(PyExc_ValueError, buf);
            for(j=0; j<4; j++)
                Py_DECREF(handseq[j]);
            return -1;
        }
        numcards = nc;
    }
    the_deal->noOfCards = numcards;

    for(i=0; i<4; i++) {
        PyObject* h = handseq[i];
        for(k=0; k<4; k++) {
            PyObject* suit = PySequence_Fast(PySequence_Fast_GET_ITEM(h, k),
                                              "each suit must be SEQ of cards");
            for(j=0; j<PySequence_Fast_GET_SIZE(suit); j++) {
                PyObject* card = PySequence_Fast_GET_ITEM(suit, j);
                int err = 0;
                if(PyString_Check(card)) {
                    int len = PyString_Size(card);
                    if (len != 1) {
                        char buf[80];
                        sprintf(buf, "cards must have length 1, not %d", len);
                        PyErr_SetString(PyExc_TypeError, buf);
                        err =1;
                    }
                } else {
                    PyErr_SetString(PyExc_TypeError, "cards must be strings");
                    err = 1;
                }
                if (!err) {
                    char* c = PyString_AS_STRING(card);
                    char* where = strchr(cardrepr, toupper(c[0]));
                    if (!where) {
                        err = 1;
                    } else {
                        int cod = 14-(where-cardrepr);
                        the_deal->deal.deal[i][k] |= 1<<cod;
                    }
                }
                if (err) {
                    for(j=0; j<4; j++)
                        Py_DECREF(handseq[j]);
                    Py_DECREF(suit);
                    return -1;
                }
            }
            Py_DECREF(suit);
        }
    }
    showdist_short(the_deal->deal.deal);

    return 0;
}
static void
deal_dealloc(PyObject *self)
{
    self->ob_type->tp_free(self);
}
/* methods */
static PyObject*
deal_str(PyObject *self)
{
    deal_cell* the_deal = (deal_cell*)self;
    int i, j, k;
    char buf[80];
    int ib=0;
    for(i=0; i<4; i++) {
        for(j=0; j<4; j++) {
            int mask = 1<<14;
            for(k=0; k<13; k++) {
                if (the_deal->deal.deal[i][j] & mask) {
                    buf[ib++] = cardrepr[k];
                }
                mask >>= 1;
            }
            if (j<3) buf[ib++] = ' ';
        }
        if (i<3) buf[ib++] = '|';
    }
    buf[ib]=0;

    return PyString_FromFormat("deal c%d %s", the_deal->noOfCards, buf);
}

static PyObject*
deal_solve(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char* nams[] = {"trump", "first", NULL};
    int trump=4, first=1;
    int i, j;

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|ii", nams, &trump, &first))
        return 0;
    deal_cell* me = (deal_cell*)self;
    struct deal d;
    d.trump = trump;
    d.first = first;
    memset(&(d.currentTrickSuit), 0, 3*sizeof(int));
    memset(&(d.currentTrickRank), 0, 3*sizeof(int));
    for(i=0; i<4; i++)
        for(j=0; j<4; j++)
            d.remainCards[i][j] = me->deal.deal[i][j];
    showdist(d.remainCards);
// printf(" 4"); fflush(0);
    struct futureTricks futp;
    int status = SolveBoard(d, -1, 1, 0, &futp);
// printf(" 5 %d", status); fflush(0);
    if (status!=1) {
        char buf[80];
        sprintf(buf, "status: %d", status);
        PyErr_SetString(PyExc_ValueError, buf);
        return 0;
    }
    PyObject *suit, *rank, *equals, *score;
    int cards = futp.cards;
    suit = PyTuple_New(cards);
    rank = PyTuple_New(cards);
    equals = PyTuple_New(cards);
    score = PyTuple_New(cards);
    for(i=0; i<cards; i++) {
        PyTuple_SET_ITEM(suit, i, PyInt_FromLong(futp.suit[i]));
        PyTuple_SET_ITEM(rank, i, PyInt_FromLong(futp.rank[i]));
        PyTuple_SET_ITEM(equals, i, PyInt_FromLong(futp.equals[i]));
        PyTuple_SET_ITEM(score, i, PyInt_FromLong(futp.score[i]));
    }
    PyObject *result = PyTuple_New(6);
    PyTuple_SET_ITEM(result, 0, PyInt_FromLong(futp.nodes));
    PyTuple_SET_ITEM(result, 1, PyInt_FromLong(futp.cards));
    PyTuple_SET_ITEM(result, 2, suit);
    PyTuple_SET_ITEM(result, 3, rank);
    PyTuple_SET_ITEM(result, 4, equals);
    PyTuple_SET_ITEM(result, 5, score);

    return result;
}

/* members */
static PyMemberDef deal_members[] = {
    {"numcards", T_INT, offsetof(deal_cell, noOfCards), READONLY, 
                 "number of cards per hand" },
    {NULL}
};

static PyMethodDef deal_methods[] = {
    {"solve", (PyCFunction)deal_solve, METH_KEYWORDS, "Solve DD play"},
    {0, 0}
};

/* Python type-object */
statichere PyTypeObject deal_type = {
    PyObject_HEAD_INIT(0)    /* initialize to 0 to ensure Win32 portability */
    0,                 /*ob_size*/
    "deal",            /*tp_name*/
    sizeof(deal_cell), /*tp_basicsize*/
    0,                 /*tp_itemsize*/
    /* methods */
    (destructor)deal_dealloc, /*tp_dealloc*/
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    deal_str,                           /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    PyObject_GenericSetAttr,            /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,
    "a bridge deal",
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    deal_methods,                       /* tp_methods */
    deal_members,                       /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    deal_init,                          /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
    PyType_GenericNew,                  /* tp_new */
    _PyObject_Del,                      /* tp_free */
    /* implied by ISO C: all zeros thereafter */
};

/* module-functions */

static PyMethodDef pydds_methods[] = {
    {0, 0}
};


/* module entry-point (module-initialization) function */
void
initpydds(void)
{
    /* Create the module and add the functions */
    PyObject *m = Py_InitModule("pydds", pydds_methods);

    /* Finish initializing the type-objects */
    PyType_Ready(&deal_type);

    PyObject_SetAttrString(m, "deal", (PyObject*)&deal_type);
}

