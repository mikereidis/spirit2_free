
    // Audio component callback API implemented by svc_svc for svc_aud

package fm.a2d.sf;

public interface svc_acb {

    // One callback function cb_audio_state()
    // To minimize the coupling between main SpiritF Service svc_svc, and it's instantiated Audio class svc_aud, should reduce to one function as in svc_tap:
    // cb_audio_key (String key, String val);
    // One function would make new additions easy; just define new key, value pairs to pass.


    // When audio state changes, call cb_audio_state() with new state value, same as com_api.audio_state:

  public abstract void cb_audio_state (String new_state);

}

    // A "Volume Changed" callback could avoid audio component svc_aud having to violate good layering principles by calling tuner functions.
    // It would move that to svc_svc which deals with both audio and tuner components.

    //public abstract void cb_audio_vol       (int new_vol);
