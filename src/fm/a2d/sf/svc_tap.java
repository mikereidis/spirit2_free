
    // Tuner component API implemented by svc_tnr for svc_svc

package fm.a2d.sf;

public interface svc_tap {

    // Now only 2 functions: tuner_get() and tuner_set(), to minimize the coupling between main SpiritF Service svc_svc, and it's instantiated Tuner class svc_tnr.
    // Two functions makes new additions easy; just define new key, value pairs to pass.
    // For 1 function, we could move to Set/Get model seen in C tuner plugin code, eg: tuner_sg()

  public abstract String tuner_get (String key);
/*
t_api_state
tuner_band
tuner_freq
tuner_pilot 
tuner_qual
tuner_rds_af
tuner_rds_af_state
tuner_rds_ct
tuner_rds_ms
tuner_rds_pi
tuner_rds_picl
tuner_rds_ps
tuner_rds_pt
tuner_rds_ptyn
tuner_rds_rt
tuner_rds_state
tuner_rds_ta
tuner_rds_taf
tuner_rds_ta_state
tuner_rds_tmc
tuner_rds_tp
tuner_rssi
tuner_seek_state
tuner_state
tuner_stereo
tuner_thresh
*/

  public abstract String tuner_set (String key, String val);
/*
t_api_state
tuner_freq
tuner_rds_af_state
tuner_rds_state
tuner_seek_state
tuner_state
tuner_stereo
*/

}

