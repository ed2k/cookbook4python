// dummy gadgets and wave
var a;
var wave = {};

var gadgets = {};
gadgets.util = {};
gadgets.util.registerOnLoadHandler = function (func) {
	//alert(func);
	func();
};
gadgets.rpc = {};
gadgets.rpc.register = function (str, state) {};
gadgets.rpc.call = function (a,b ,str, state) {
  //alert(state);
};

wave.Callback = function(b, c) {
  this.callback_ = b;
  this.context_ = c || null
};
wave.Callback.prototype.invoke = function() {
  if(this.callback_) {
    var b = Array.prototype.slice.call(wave.Callback.prototype.invoke.arguments, 0);
    this.callback_.apply(this.context_, b)
  }
};
wave.Mode = {UNKNOWN:0, VIEW:1, EDIT:2, DIFF_ON_OPEN:3, PLAYBACK:4};
wave.PARAM_NAME_ = "wave";
wave.viewer_ = null;
wave.host_ = null;
wave.participants_ = [];
wave.participantMap_ = {};
wave.participantCallback_ = new wave.Callback(null);
wave.state_ = null;
wave.stateCallback_ = new wave.Callback(null);
wave.mode_ = null;
wave.modeCallback_ = new wave.Callback(null);
wave.inWaveContainer_ = true;
wave.util = wave.util || {};
wave.util.SPACES = "                                                 ";
wave.util.toSpaces_ = function(b) {
  return wave.util.SPACES.substring(0, b * 2)
};
wave.util.isArray_ = function(b) {
  try {
    return b && typeof b.length == "number"
  }catch(c) {
    return false
  }
};
wave.util.printJson = function(b, c, e) {
  if(!b || typeof b.valueOf() != "object") {
    if(typeof b == "string")return"'" + b + "'";
    else if(b instanceof Function)return"[function]";
    return"" + b
  }var d = [], f = wave.util.isArray_(b), g = f ? "[]" : "{}", h = c ? "\n" : "", k = c ? " " : "", l = 0;
  e = e || 1;
  c || (e = 0);
  d.push(g.charAt(0));
  for(var i in b) {
    var j = b[i];
    l++ > 0 && d.push(", ");
    if(f)d.push(wave.util.printJson(j, c, e + 1));
    else {
      d.push(h);
      d.push(wave.util.toSpaces_(e));
      d.push(i + ": ");
      d.push(k);
      d.push(wave.util.printJson(j, c, e + 1))
    }
  }if(!f) {
    d.push(h);
    d.push(wave.util.toSpaces_(e - 1))
  }d.push(g.charAt(1));
  return d.join("")
};
wave.Participant = function(b, c, e) {
  this.id_ = b || "";
  this.displayName_ = c || "";
  this.thumbnailUrl_ = e || ""
};
wave.Participant.prototype.getId = function() {
  return this.id_
};
wave.Participant.prototype.getDisplayName = function() {
  return this.displayName_
};
wave.Participant.prototype.getThumbnailUrl = function() {
  return this.thumbnailUrl_
};
wave.Participant.fromJson_ = function(b) {
  var c = new wave.Participant;
  c.id_ = b.id;
  c.displayName_ = b.displayName;
  c.thumbnailUrl_ = b.thumbnailUrl;
  return c
};
wave.State = function() {
  this.state_ = null;
};
a = wave.State.prototype;
a.get = function(b, c) {
  return this.state_[b] || c || null
};
a.getKeys = function() {
  var b = [];
  for(var c in this.state_)b.push(c);
  return b
};
a.submitDelta = function(b) {
  alert(typeof b);
  gadgets.rpc.call(null, "wave_gadget_state", null, b)
  wave.receiveState_(b);

};
a.submitValue = function(b, c) {
  var e = {};
  e[b] = c;
  this.submitDelta(e)
};
a.reset = function() {
  var b = {};
  for(var c in this.state_)b[c] = null;
  this.submitDelta(b)
};
a.setState_ = function(b) {
  this.state_ = b || {}
};
a.toString = function() {
  return wave.util.printJson(this.state_, true)
};
wave.checkWaveContainer_ = function() {
  //var b = gadgets.util.getUrlParameters();
  wave.inWaveContainer_ = true;//b.hasOwnProperty(wave.PARAM_NAME_) && b[wave.PARAM_NAME_]
};
wave.isInWaveContainer = function() {
  return wave.inWaveContainer_
};
wave.receiveWaveParticipants_ = function(b) {
  wave.viewer_ = null;
  wave.host_ = null;
  wave.participants_ = [];
  wave.participantMap_ = {};
  var c = b.myId, e = b.authorId;
  b = b.participants;
  for(var d in b) {
    var f = wave.Participant.fromJson_(b[d]);
    if(d == c)wave.viewer_ = f;
    if(d == e)wave.host_ = f;
    wave.participants_.push(f);
    wave.participantMap_[d] = f
  }
  if(!wave.viewer_ && c) {
    f = new wave.Participant(c, c);
    wave.viewer_ = f;
    wave.participants_.push(f);
    wave.participantMap_[c] = f
  }
  wave.participantCallback_.invoke(wave.participants_)
};
wave.receiveState_ = function(b) {
  wave.state_ = wave.state_ || new wave.State;
  wave.state_.setState_(b);
  wave.stateCallback_.invoke(wave.state_)
};
wave.receiveMode_ = function(b) {
  wave.mode_ = b || {};
  wave.modeCallback_.invoke(wave.getMode())
};
wave.getViewer = function() {
  return wave.viewer_
};
wave.getHost = function() {
  return wave.host_
};
wave.getParticipants = function() {
  return wave.participants_
};
wave.getParticipantById = function(b) {
  return wave.participantMap_[b]
};
wave.getState = function() {
  return wave.state_
};
wave.getMode = function() {
  if(wave.mode_) {
    var b = wave.mode_["${playback}"], c = wave.mode_["${edit}"];
    if(b != null && c != null)return b == "1" ? wave.Mode.PLAYBACK : c == "1" ? wave.Mode.EDIT : wave.Mode.VIEW
  }return wave.Mode.UNKNOWN
};
wave.isPlayback = function() {
  var b = wave.getMode();
  return b == wave.Mode.PLAYBACK || b == wave.Mode.UNKNOWN
};
wave.setStateCallback = function(b, c) {
  wave.stateCallback_ = new wave.Callback(b, c);
  wave.state_ && wave.stateCallback_.invoke(wave.state_);
  //b();
};
wave.setParticipantCallback = function(b, c) {
  wave.participantCallback_ = new wave.Callback(b, c);
  wave.participants_ && wave.participantCallback_.invoke(wave.participants_);
  b();
};
wave.setModeCallback = function(b, c) {
  wave.modeCallback_ = new wave.Callback(b, c);
  wave.mode_ && wave.modeCallback_.invoke(wave.getMode())
};
wave.getTime = function() {
  return(new Date).getTime()
};
wave.log = function(b) {
  gadgets.rpc.call(null, "wave_log", null, b || "")
};
wave.internalInit_ = function() {
  wave.checkWaveContainer_();
  if(wave.isInWaveContainer()) {
    gadgets.rpc.register("wave_participants", wave.receiveWaveParticipants_);
    gadgets.rpc.register("wave_gadget_state", wave.receiveState_);
    gadgets.rpc.register("wave_gadget_mode", wave.receiveMode_);
    gadgets.rpc.call(null, "wave_enable", null, "0.1")
  }
};
(wave.init_ = function() {
  window.gadgets && gadgets.util.registerOnLoadHandler(function() {
    wave.internalInit_()
  })
})();

//------- give default value for testing
// so far able to update to initial state, with a deck of cards.
wave.viewer_ = new wave.Participant('id','dname','url');
wave.participants_.push(wave.viewer_);
wave.state_ = new wave.State();


function fillup() {
  //alert("");
  wave.stateCallback_(); // to do, the delta???, 2. not updating already updated
  window.setTimeout("fillup()",5000);
}
window.setTimeout("fillup()",5000);
