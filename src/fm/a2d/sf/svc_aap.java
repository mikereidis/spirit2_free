
    // Audio API

package fm.a2d.sf;

public interface svc_aap {
  public abstract String audio_sessid_get       ();
  public abstract String audio_state_set        (String new_state);
  public abstract String audio_mode_set         (String new_audio_mode);
  public abstract String audio_output_set       (String new_audio_output, boolean start_set);
  public abstract String audio_stereo_set       (String new_audio_stereo);
  public abstract String audio_record_state_set (String new_state);
  public abstract String audio_digital_amp_set  ();
}

