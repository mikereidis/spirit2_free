
#define  NO_RDS

            /*
             * Interface file between the vendor specific drivers and the fmradio
             * jni layer. The vendor driver need to define register function with
             * name defined by FMRADIO_REGISTER_FUNC of type fmradio_reg_func_t that
             * will return a pointer to a signature (FMRADIO_SIGNATURE) to make sure
             * it executed correctly and fill struct fmradio_vendor_methods with
             * functions implementing functions (or NULL if not supported).
             */    

            #ifndef ANDROID_FMRADIO_INTERFACE_H
            #define ANDROID_FMRADIO_INTERFACE_H

#define bool    int
#define pthread_mutex_t int
#define pthread_cond_t int
#define false   0
#define true    1



            __BEGIN_DECLS

#define FMRADIO_REGISTER_FUNC "register_fmradio_functions"

            #define FMRADIO_SIGNATURE 0xDEADBABE

            #define FMRADIO_CAPABILITY_RECEIVER 0x0001
            #define FMRADIO_CAPABILITY_TRANSMITTER 0x0002
            #define FMRADIO_CAPABILITY_TUNER_WRAP_AROUND 0x0004
            #define FMRADIO_CAPABILITY_RDS_SUPPORTED 0x0008

            /*
             * return values. Not defined as enum since some functions either
             * return a positive value or these codes, like getFrequency.
             */
            #define  FMRADIO_OK 0
            #define  FMRADIO_INVALID_STATE -1       /* internally in jni layer */
            #define  FMRADIO_UNSUPPORTED_OPERATION -2
            #define  FMRADIO_IO_ERROR -3
            #define  FMRADIO_INVALID_PARAMETER -4
            #define  FMRADIO_FORCED_RESET -5

            /* RDS */
            #define RDS_MAX_AFS 25
            #define RDS_PSN_MAX_LENGTH 8
            #define RDS_RT_MAX_LENGTH 64
            #define RDS_CT_MAX_LENGTH 14
            #define RDS_PTYN_MAX_LENGTH 8
            #define RDS_NUMBER_OF_TMC 3

            enum fmradio_band_t {
                FMRADIO_BAND_US,
                FMRADIO_BAND_EU,
                FMRADIO_BAND_JAPAN,
                FMRADIO_BAND_CHINA
            };

            enum fmradio_seek_direction_t {
                FMRADIO_SEEK_DOWN,
                FMRADIO_SEEK_UP
            };

            enum fmradio_reset_reason_t {
                FMRADIO_RESET_NON_CRITICAL = 0,
                FMRADIO_RESET_CRITICAL,
                FMRADIO_RESET_OTHER_IN_USE,    /* internally in jni layer */
                FMRADIO_RESET_RADIO_FORBIDDEN, /* internally in java layer */
            };

            enum fmradio_extra_command_type_t {
                FMRADIO_TYPE_INT,
                FMRADIO_TYPE_STRING
            };

            enum fmradio_switch_reason_t {
                FMRADIO_SWITCH_AF,
                FMRADIO_SWITCH_TA,
                FMRADIO_SWITCH_TA_END
            };

            union fmradio_extra_data_t {
                int int_value;
                char *string_value;
            };

            struct fmradio_rds_bundle_t {
                unsigned short pi;
                short tp;
                short pty;
                short ta;
                short ms;
                short num_afs;
                int af[RDS_MAX_AFS];
                char psn[RDS_PSN_MAX_LENGTH + 1];
                char rt[RDS_RT_MAX_LENGTH + 1];
                char ct[RDS_CT_MAX_LENGTH + 1];
                char ptyn[RDS_PTYN_MAX_LENGTH + 1];
                short tmc[RDS_NUMBER_OF_TMC];
                int taf;
            };

            struct fmradio_extra_command_ret_item_t {
                char *key;
                enum fmradio_extra_command_type_t type;
                union fmradio_extra_data_t data;
            };

            /* vendor callbacks only for RX */
            struct fmradio_vendor_callbacks_t {
                void (*on_playing_in_stereo_changed) (int is_stereo);
                void (*on_rds_data_found) (struct fmradio_rds_bundle_t * rds_bundle,
                                           int frequency);
                void (*on_signal_strength_changed) (int new_level);
                void (*on_automatic_switch) (int new_freq,
                                             enum fmradio_switch_reason_t reason);
                void (*on_forced_reset) (enum fmradio_reset_reason_t reason);
            };
            struct fmradio_vendor_methods_t {
                int (*rx_start) (void ** session_data,
                                const struct fmradio_vendor_callbacks_t * callbacks,
                                int low_freq, int high_freq, int default_freq, int grid);
                int (*tx_start) (void ** session_data,
                                const struct fmradio_vendor_callbacks_t * callbacks,
                                int low_freq, int high_freq, int default_freq, int grid);
                int (*pause) (void ** session_data);
                int (*resume) (void ** session_data);
                int (*reset) (void ** session_data);
                int (*set_frequency) (void ** session_data, int frequency);
                int (*get_frequency) (void ** session_data);
                int (*stop_scan) (void ** session_data);
                int (*send_extra_command) (void ** session_data, const char * command,
                                         char ** parameters,
                                         struct fmradio_extra_command_ret_item_t ** out_parameters);
                /* rx only */
                int (*scan) (void ** session_data, enum fmradio_seek_direction_t direction);

                int (*full_scan) (void ** session_data, int ** found_freqs,
                                 int ** signal_strenghts);
                int (*get_signal_strength) (void ** session_data);
                int (*is_playing_in_stereo) (void ** session_data);
                int (*is_rds_data_supported) (void ** session_data);
                int (*is_tuned_to_valid_channel) (void ** session_data);
                int (*set_automatic_af_switching) (void ** session_data, int automatic);
                int (*set_automatic_ta_switching) (void ** session_data, int automatic);
                int (*set_force_mono) (void ** session_data, int force_mono);
                int (*get_threshold) (void ** session_data);
                int (*set_threshold) (void ** session_data, int threshold);
                int (*set_rds_reception) (void ** session_data, int use_rds);
                /* tx only */
                int (*block_scan) (void ** session_data, int low_freq, int high_freq,
                                  int ** found_freqs, int ** signal_strenghts);
                int (*set_rds_data) (void ** session_data, char * key, void * value);
            };

            typedef int (*fmradio_reg_func_t) (unsigned int * signature_p,
                                               struct fmradio_vendor_methods_t * vendor_funcs_p);

            __END_DECLS

            #endif  // ANDROID_FMRADIO_INTERFACE_H;

