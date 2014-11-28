
    // Audio API

package fm.a2d.sf;

public interface svc_aap {

  public abstract String audio_sessid_get ();

  public abstract String audio_state_set (String state);

  public abstract String audio_output_set (String audio_output);

  public abstract String audio_stereo_set (String new_audio_stereo);

  public abstract boolean audio_blank_get ();
  public abstract boolean audio_blank_set (boolean blank);

}

