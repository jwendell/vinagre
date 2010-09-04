const Tp = imports.gi.TelepathyGLib;
const Presence = Tp.ConnectionPresenceType;
const Lang = imports.lang;
const _ = imports.gettext.gettext;

var am = null;
var ready = false;

function ImStatus () {
  this._init ();
}

ImStatus.prototype = {
  _init: function () {
    this.previous_state = 0;
    this.signal_id = 0;
  },

  save_state: function () {
    var status = {};
    this.previous_state = am.get_most_available_presence ();
    if ([Presence.AVAILABLE, Presence.AWAY, Presence.BUSY, Presence.EXTENDED_AWAY].indexOf (this.previous_state) >= 0)
      ready = true;
  },

  set_state: function (busy) {
    if (!ready)
      return;

    var status = {};
    var message = {};
    var state = busy ? Presence.BUSY : this.previous_state;
    var new_status = busy ? "busy" : "";

    am.get_most_available_presence (status, message);
    am.set_all_requested_presences (state, new_status, message.value);
  },

  activate: function () {
   if (this._initAccountManager ())
     this.signal_id = this.object.signal.window_state_event.connect (Lang.bind (this, this._windowStateChanged));
   else
     print (_("Could not communicate to Telepathy. IM Status plugin will not work."));
  },

  deactivate: function () {
    this.set_state (false);
    if (this.signal_id > 0)
      this.object.signal.disconnect (this.signal_id);
  },

  _initAccountManager: function () {
    if (am == null) {
       am = Tp.AccountManager.dup ();
       if (!am)
         return false;

       am.prepare_async (null, Lang.bind (this,
         function (o, r) {
           am.prepare_finish (r);
           this._changeImStatus ();
         }));
    }
    return true;
  },

  _windowStateChanged: function (window, e) {
    if (e.window_state.changed_mask & imports.gi.Gdk.WindowState.FULLSCREEN)
      this._changeImStatus ();
    return false;
  },

  _changeImStatus: function () {
    if (this.object.window.get_state () & imports.gi.Gdk.WindowState.FULLSCREEN) {
      this.save_state ();
      this.set_state (true);
    } else {
      this.set_state (false);
    }
  }
}

var extensions = {
  'PeasActivatable': new ImStatus ()
};
