#include "easy_nmea_server.h"

void readConfig(char *cfile)
{

    int i = 0, hcnt = 0;

    config_t cfg, *cf;
    const config_setting_t *sections = NULL, *source_hosts = NULL;
    int count = 0, shosts_count = 0, sz = 255;
    char config_msg[sz];
    memset(config_msg, '\0', sz);
    const char * shost = NULL;
    char * resolved = NULL;

    const char * customlog = NULL;
    const char * debuglog = NULL;
    const char * conlog = NULL;
    const char * accesslog = NULL;
    //

    const char *t_longitude;
    const char *t_latitude;
    const char *t_method;
    const char *satellite;
    const char *faa_mode;
    const char *sentence;
    const char *data_status;
    //
    enmea_int id;

    int maxcon = 0;
    //

    i = 0;
    cf = &cfg;
    config_init(cf);

    if (CONFIG_FALSE == config_read_file(cf, cfile))
    {
        printf("Something wrong in %s, line #%d - %s. Exiting...\n",
               cfile,
               config_error_line(cf) - 1,
               config_error_text(cf));
        config_destroy(cf);

        exit(EXIT_FAILURE);
    }
    else
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Reading configuration file '%s'", cfile);
        writeToCustomLog(config_msg);
    }

    if (!config_lookup_int(cf, "maxcon", &maxcon))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of maxcon");
        writeToCustomLog(config_msg);
    }
    else
    {
        globals.maxcon = maxcon;
    }



    if (globals.maxcon > SECTIONS_MAX_COUNT * MAX_CON_PER_IP)
    {
        globals.maxcon = SECTIONS_MAX_COUNT * MAX_CON_PER_IP;
    }


    if (!config_lookup_bool(cf, "use-resolver", &globals.use_resolver))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of use-resolver");
        writeToCustomLog(config_msg);
    }


    if (!config_lookup_bool(cf, "log-to-syslog", &globals.use_syslog))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of log-to-syslog");
        writeToCustomLog(config_msg);
    }

    customlog = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "custom_log", &customlog))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of custom_log");
        writeToCustomLog(config_msg);
    }
    else
    {
        globals.custom_logfile_name = xrealloc(globals.custom_logfile_name, (sizeof (char) * (strlen(customlog) + 1)));
        strcpy(globals.custom_logfile_name, customlog);
        globals.custom_logfile_name[strlen(customlog)] = '\0';
        customlog = NULL;
    }

    debuglog = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "debug_log", &debuglog))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of debug_log");
        writeToCustomLog(config_msg);
    }
    else
    {
        globals.debug_logfile_name = xrealloc(globals.debug_logfile_name, (sizeof (char) * strlen(debuglog) + 1));
        strcpy(globals.debug_logfile_name, debuglog);
        globals.debug_logfile_name[strlen(debuglog)] = '\0';
        debuglog = NULL;
    }

    conlog = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "connections_log", &conlog))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of connections_log");
        writeToCustomLog(config_msg);
    }
    else
    {
        globals.connections_logfile_name = xrealloc(globals.connections_logfile_name, (sizeof (char) * strlen(conlog) + 1));
        strcpy(globals.connections_logfile_name, conlog);
        globals.connections_logfile_name[strlen(conlog)] = '\0';
        conlog = NULL;
    }

    accesslog = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "access_log", &accesslog))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "%s", "Using default value of access_log");
        writeToCustomLog(config_msg);
    }
    else
    {
        globals.access_logfile_name = xrealloc(globals.access_logfile_name, (sizeof (char) * strlen(accesslog) + 1));
        strcpy(globals.access_logfile_name, accesslog);
        globals.access_logfile_name[strlen(accesslog)] = '\0';
        accesslog = NULL;
    }


    sections = config_lookup(cf, "clients");
    if (NULL == sections)
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Undefined section '%s'. Exiting.", "clients");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    //
    count = config_setting_length(sections);
    if (count > SECTIONS_MAX_COUNT)count = SECTIONS_MAX_COUNT;
    //
    globals.sections_cnt = count;
    //
    memset(config_msg, '\0', sz);
    snprintf(config_msg, sz, "Connections count restricted to %d", globals.maxcon);
    writeToCustomLog(config_msg);
    //
    memset(config_msg, '\0', sz);
    snprintf(config_msg, sz, "Configurations count: %d", globals.sections_cnt);
    writeToCustomLog(config_msg);
    //

    //
    for (i = 0; i < count; ++i)
    {
        config_setting_t *section = config_setting_get_elem(sections, i);

        ////////////////////////////////////////////////////////////////////////////////
        if (!config_setting_lookup_int(section, "id", &id))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'id' for client");
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            globals.sections[i].id = id;
        }
        //
        if (!config_setting_lookup_bool(section, "enabled", &globals.sections[i].enabled))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'enabled' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        //
        if (!globals.sections[i].enabled)continue;
        //
        if (!config_setting_lookup_bool(section, "checksum", &globals.sections[i].checksum))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'checksum' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        //
        source_hosts = config_setting_get_member(section, "source-hosts");

        if (NULL != source_hosts && CONFIG_TRUE == config_setting_is_array(source_hosts))
        {
            shosts_count = config_setting_length(source_hosts);
            for (hcnt = 0; hcnt < shosts_count; hcnt++)
            {

                shost = config_setting_get_string_elem(source_hosts, hcnt);
                if (0 == strcmp(shost, ""))
                {
                    shost = "0.0.0.0";
                    snprintf(globals.sections[i].from[hcnt].address, 16, "%s", shost);
                    snprintf(globals.sections[i].from[hcnt].netmask, 16, "%s", "");
                    globals.sections[i].from[hcnt].lastoctet_range = 0;

                }
                else
                {
                    if (isSubnet(shost))
                    {
                        char * tmp;
                        tmp = strdup(shost);
                        char * address = strtok(tmp, "/");
                        char * netmask = strtok(NULL, "/");
                        snprintf(globals.sections[i].from[hcnt].address, 16, "%s", (NULL != (resolved = nslookup((char *) address)) ? resolved : address));
                        snprintf(globals.sections[i].from[hcnt].netmask, 16, "%s", netmask);
                        globals.sections[i].from[hcnt].lastoctet_range = 0;

                    }
                    else if (isIpRange(shost))
                    {
                        char *tmp;
                        tmp = strdup(shost);
                        char * address = strtok(tmp, "-");
                        char * range = strtok(NULL, "-");
                        snprintf(globals.sections[i].from[hcnt].address, 16, "%s", (NULL != (resolved = nslookup((char *) address)) ? resolved : address));
                        snprintf(globals.sections[i].from[hcnt].netmask, 16, "%s", "");
                        globals.sections[i].from[hcnt].lastoctet_range = atoi(range);

                    }
                    else
                    {
                        /* assuming we have an ordinary hostname now */
                        snprintf(globals.sections[i].from[hcnt].address, 16, "%s", (NULL != (resolved = nslookup((char *) shost)) ? resolved : shost));
                        snprintf(globals.sections[i].from[hcnt].netmask, 16, "%s", "");
                        globals.sections[i].from[hcnt].lastoctet_range = 0;

                    }
                }

            }

        }
        else
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Undefined subsection '%s' or syntax error . Exiting...", "source_hosts");
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }

        if (!config_setting_lookup_bool(section, "simulate", &globals.sections[i].simulate))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'simulate' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }

        t_longitude = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "longitude", &t_longitude))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'longitude' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            memset(globals.sections[i].longitude, '\0', 14);
            snprintf(globals.sections[i].longitude, 14, "%s", ValidateLongitude(t_longitude));
            t_longitude = NULL;
        }

        t_latitude = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "latitude", &t_latitude))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'latitude' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            memset(globals.sections[i].latitude, '\0', 13);
            snprintf(globals.sections[i].latitude, 13, "%s", ValidateLatitude(t_latitude));
            t_latitude = NULL;
        }

        t_method = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "method", &t_method))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'method' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            if (0 != strcasecmp(t_method, ""))
            {
                if (0 == strcasecmp(t_method, "increment"))
                {
                    globals.sections[i].sim_methods = INCREMENT;
                }
                else if (0 == strcasecmp(t_method, "decrement"))
                {
                    globals.sections[i].sim_methods = DECREMENT;
                }
                else if (0 == strcasecmp(t_method, "shuffle"))
                {
                    globals.sections[i].sim_methods = SHUFFLE;
                }
            }
            else
            {
                globals.sections[i].sim_methods = INCREMENT;
            }
            t_method = NULL;
        }

        if (!config_setting_lookup_float(section, "speed", &globals.sections[i].speed))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'speed' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }

        satellite = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "satellite", &satellite))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'satellite' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            if (0 != strcasecmp(satellite, ""))
            {
                if (0 == strcasecmp(satellite, "gps"))
                {
                    globals.sections[i].navigator_sat = GPS;
                }
                else if (0 == strcasecmp(satellite, "glonass"))
                {
                    globals.sections[i].navigator_sat = GLONASS;
                }
                else if (0 == strcasecmp(satellite, "both"))
                {
                    globals.sections[i].navigator_sat = BOTH;
                }
            }
            else
            {
                globals.sections[i].navigator_sat = BOTH;
            }
            satellite = NULL;
        }

        data_status = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "data_status", &data_status))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'data_status' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            if (0 != strcasecmp(data_status, ""))
            {
                if (0 == strcasecmp(data_status, "valid"))
                {
                    globals.sections[i].data_status = A_VALID;
                }
                else if (0 == strcasecmp(data_status, "invalid"))
                {
                    globals.sections[i].data_status = V_INVALID;
                }
            }
            else
            {
                globals.sections[i].data_status = A_VALID;
            }
            data_status = NULL;
        }


        sentence = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "sentence", &sentence))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'sentence' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            if (0 != strcasecmp(sentence, ""))
            {

                if (0 == strcasecmp(sentence, "GLL"))
                {
                    globals.sections[i].sentence = GLL;
                }
                else if (0 == strcasecmp(sentence, "RMC"))
                {
                    globals.sections[i].sentence = RMC;
                }
                else if (0 == strcasecmp(sentence, "VTG"))
                {
                    globals.sections[i].sentence = VTG;
                }
                else if (0 == strcasecmp(sentence, "ZDA"))
                {
                    globals.sections[i].sentence = ZDA;
                }
                else if (0 == strcasecmp(sentence, "GGA"))
                {
                    globals.sections[i].sentence = GGA;
                }
                else if (0 == strcasecmp(sentence, "GSA"))
                {
                    globals.sections[i].sentence = GSA;
                }
                else if (0 == strcasecmp(sentence, "GSV"))
                {
                    globals.sections[i].sentence = GSV;
                }
                else if (0 == strcasecmp(sentence, "XTE"))
                {
                    globals.sections[i].sentence = XTE;
                }
                else if (0 == strcasecmp(sentence, "RMB"))
                {
                    globals.sections[i].sentence = RMB;
                }
                else if (0 == strcasecmp(sentence, "DTM"))
                {
                    globals.sections[i].sentence = DTM;
                }
                else
                {
                    memset(config_msg, '\0', sz);
                    snprintf(config_msg, sz, "Unknown or unsupported 'sentence' for client [id:%d]", globals.sections[i].id);
                    writeToCustomLog(config_msg);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                globals.sections[i].sentence = GLL;
            }
            sentence = NULL;
        }

        faa_mode = xmalloc(sizeof (char) * CFG_PARAM_LEN);
        if (!config_setting_lookup_string(section, "faa_mode", &faa_mode))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Can not find 'faa_mode' for client [id:%d]", globals.sections[i].id);
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
        else
        {
            if (0 != strcasecmp(faa_mode, ""))
            {
                if (0 == strcasecmp(faa_mode, "A"))
                {
                    globals.sections[i].sentence = A;
                }
                else if (0 == strcasecmp(faa_mode, "D"))
                {
                    globals.sections[i].sentence = D;
                }
                else if (0 == strcasecmp(faa_mode, "E"))
                {
                    globals.sections[i].sentence = E;
                }
                else if (0 == strcasecmp(faa_mode, "M"))
                {
                    globals.sections[i].sentence = M;
                }
                else if (0 == strcasecmp(faa_mode, "S"))
                {
                    globals.sections[i].sentence = S;
                }
                else if (0 == strcasecmp(faa_mode, "N"))
                {
                    globals.sections[i].sentence = N;
                }

                else
                {
                    memset(config_msg, '\0', sz);
                    snprintf(config_msg, sz, "Unknown or unsupported 'faa_mode' for client [id:%d]", globals.sections[i].id);
                    writeToCustomLog(config_msg);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                globals.sections[i].sentence = GLL;
            }
            faa_mode = NULL;
        }


    } /* for loop */

    config_destroy(cf);

}

char * ValidateLatitude(const char *latitude)
{
    int sz = 255;
    char config_msg[sz];

    char *lati = NULL;
    char *minutes = NULL;
    char *seconds = NULL;
    char *tmp = NULL;
    char *S_N = NULL;

    char returnval[13];

    char *lat = strdup(latitude);
    char delimiter[2] = ".";
    /* get the first token */
    lati = strtok(lat, delimiter);

    if (NULL == lati)
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Invalid 'latitude'");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    minutes = strtok(NULL, delimiter);
    tmp = strtok(NULL, delimiter);
    seconds = strtok(tmp, ",");
    if (NULL == seconds)
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Invalid 'latitude'");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    S_N = strtok(NULL, ",");

    if (90 < atoi(lati))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Latitude can not be greater than 90");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }

    if (59 < atoi(minutes))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Minute can not be greater than 59");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }

    if (59999 < atoi(seconds))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Seconds can not be greater than 59");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }

    if (0 != strcasecmp(S_N, "s") && 0 != strcasecmp(S_N, "n"))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Latitude should be S or N");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    memset(returnval, '\0', 13);
    upper_string(S_N);
    snprintf(returnval, 13, "%s%s.%s,%s", lati, minutes, seconds,S_N);
    strncpy(return_latitude, returnval, 12);
    FREE(lat);
    minutes = NULL;
    seconds = NULL;
    tmp = NULL;
    return return_latitude;
}

char * ValidateLongitude(const char *longitude)
{

    int sz = 255;
    char config_msg[sz];

    char *longi = NULL;
    char *minutes = NULL;
    char *seconds = NULL;
    char *tmp = NULL;
    char *W_E = NULL;

    char returnval[14];

    char *longit = strdup(longitude);
    char delimiter[2] = ".";
    /* get the first token */
    longi = strtok(longit, delimiter);

    if (NULL == longi)
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Invalid 'longitude'");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    minutes = strtok(NULL, delimiter);
    tmp = strtok(NULL, delimiter);
    seconds = strtok(tmp, ",");
    if (NULL == seconds)
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Invalid 'longitude'");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    W_E = strtok(NULL, ",");

    if (180 < atoi(longi))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Longitude can not be greater than 180");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }

    if (59 < atoi(minutes))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Minute can not be greater than 59");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }

    if (59999 < atoi(seconds))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Seconds can not be greater than 59");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }

    if (0 != strcasecmp(W_E, "w") && 0 != strcasecmp(W_E, "e"))
    {
        memset(config_msg, '\0', sz);
        snprintf(config_msg, sz, "Longitude should be W or E");
        writeToCustomLog(config_msg);
        exit(EXIT_FAILURE);
    }
    else
    {
        if (1 < strlen(W_E))
        {
            memset(config_msg, '\0', sz);
            snprintf(config_msg, sz, "Longitude should be W or E");
            writeToCustomLog(config_msg);
            exit(EXIT_FAILURE);
        }
    }
    memset(returnval, '\0', 14);
    upper_string(W_E);
    snprintf(returnval, 14, "%s%s.%s,%s", longi, minutes, seconds, W_E);
    strncpy(return_longitude, returnval, 13);
    FREE(longit);
    minutes = NULL;
    seconds = NULL;
    tmp = NULL;
    return return_longitude;
}