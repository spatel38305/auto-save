#include <yed/plugin.h>

void auto_save( yed_event *ye );

unsigned int save_frequency;

unsigned long long last_time;

int yed_plugin_boot( yed_plugin *self )
{
    YED_PLUG_VERSION_CHECK();

    yed_event_handler yeh;
    yeh.kind = EVENT_POST_PUMP;
    yeh.fn = auto_save;

    yed_plugin_add_event_handler( self, yeh );

    save_frequency = 30000;
    last_time = measure_time_now_ms();

    if ( yed_get_var( "auto-save-frequency-ms" ) == NULL )
    {
        yed_set_var( "auto-save-frequency-ms", "30000" );
    }
    else
    {
        yed_get_var_as_int( "auto-save-frequency-ms", &save_frequency );

        if ( save_frequency < 10000 )
        {
            save_frequency = 10000;
            yed_set_var( "auto-save-frequency-ms", "10000" );
        }
    }

    return 0;
}

void auto_save( yed_event *ye )
{
    unsigned long long now = measure_time_now_ms();

    if ( ( now - last_time ) >= save_frequency )
    {
        yed_buffer *buff;
        tree_it( yed_buffer_name_t, yed_buffer_ptr_t ) bit;
        int status;
        char *pretty_path;
        char *path;
        char exp_path[4096];

        tree_traverse( ys->buffers, bit )
        {
            buff = tree_it_val( bit );

            if ( !( buff->flags & BUFF_SPECIAL ) )
            {
                if ( buff->flags & BUFF_MODIFIED )
                {
                    pretty_path = buff->name;
                    relative_path_if_subtree( buff->name, exp_path );
                    path = exp_path;

                    status = yed_write_buff_to_file( buff, path );

                    switch ( status ) {
                        case BUFF_WRITE_STATUS_ERR_DIR:
                            yed_cerr( "did not write to '%s' -- path is a directory", pretty_path );
                            break;
                        case BUFF_WRITE_STATUS_ERR_PER:
                            yed_cerr( "did not write to '%s' -- permission denied", pretty_path );
                            break;
                        case BUFF_WRITE_STATUS_ERR_UNK:
                            yed_cerr( "did not write to '%s' -- unknown error", pretty_path );
                            break;
                        case BUFF_WRITE_STATUS_SUCCESS:
                            yed_cprint( "wrote to '%s'", pretty_path );
                            break;
                    }
                }
            }
        }

        last_time = now;
    }
}
