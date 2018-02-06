// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_uamqp_c/uamqp.h"
#include "azure_uamqp_c/amqp_management.h"

/* This sample connects to an Event Hub, authenticates using SASL PLAIN (key name/key) and then it received all messages for partition 0 */
/* Replace the below settings with your own.*/
#define EH_HOST "<<<Replace with your own EH host (like myeventhub.servicebus.windows.net)>>>"
#define EH_KEY_NAME "<<<Replace with your own key name>>>"
#define EH_KEY "<<<Replace with your own key>>>"
#define EH_NAME "<<<Insert your event hub name here>>>"

static bool opened = false;

static AMQP_VALUE on_message_received(const void* context, MESSAGE_HANDLE message)
{
    (void)context;
    (void)message;

    (void)printf("Message received.\r\n");

    return messaging_delivery_accepted();
}

static void on_amqp_management_open_complete(void* context, AMQP_MANAGEMENT_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;

    opened = true;
}

static void on_amqp_management_error(void* context)
{
    (void)context;
}

static void on_read_operation_complete(void* context, AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT execute_operation_result, unsigned int status_code, const char* status_description)
{
    (void)context;
    (void)execute_operation_result;
    (void)status_code;
    (void)status_description;
}

int main(int argc, char** argv)
{
    int result;

    XIO_HANDLE sasl_io = NULL;
    CONNECTION_HANDLE connection = NULL;
    SESSION_HANDLE session = NULL;

    (void)argc;
    (void)argv;

    if (platform_init() != 0)
    {
        result = -1;
    }
    else
    {
        size_t last_memory_used = 0;
        SASL_PLAIN_CONFIG sasl_plain_config;
        SASL_MECHANISM_HANDLE sasl_mechanism_handle;
        TLSIO_CONFIG tls_io_config;
        const IO_INTERFACE_DESCRIPTION* tlsio_interface;
        XIO_HANDLE tls_io;
        SASLCLIENTIO_CONFIG sasl_io_config;

        gballoc_init();

        /* create SASL plain handler */
        sasl_plain_config.authcid = EH_KEY_NAME;
        sasl_plain_config.authzid = NULL;
        sasl_plain_config.passwd = EH_KEY;

        sasl_mechanism_handle = saslmechanism_create(saslplain_get_interface(), &sasl_plain_config);

        /* create the TLS IO */
        tls_io_config.hostname = EH_HOST;
        tls_io_config.port = 5671;
        tls_io_config.underlying_io_interface = NULL;
        tls_io_config.underlying_io_parameters = NULL;

        tlsio_interface = platform_get_default_tlsio();
        tls_io = xio_create(tlsio_interface, &tls_io_config);

        /* create the SASL client IO using the TLS IO */
        sasl_io_config.underlying_io = tls_io;
        sasl_io_config.sasl_mechanism = sasl_mechanism_handle;
        sasl_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config);

        /* create the connection, session and link */
        connection = connection_create(sasl_io, EH_HOST, "whatever", NULL, NULL);
        connection_set_trace(connection, true);
        session = session_create(connection, NULL, NULL);

        /* set incoming window to 100 for the session */
        session_set_incoming_window(session, 100);

        // create the AMQP management handle
        AMQP_MANAGEMENT_HANDLE amqp_management = amqp_management_create(session, "$management");
        amqp_management_open_async(amqp_management, on_amqp_management_open_complete, amqp_management, on_amqp_management_error, amqp_management);

        while (!opened)
        {
            connection_dowork(connection);
        }

        MESSAGE_HANDLE message = message_create();
        message_set_body_amqp_value(message, amqpvalue_create_null());
        AMQP_VALUE properties_map = amqpvalue_create_map();
        AMQP_VALUE name_key = amqpvalue_create_string("name");
        AMQP_VALUE name_value = amqpvalue_create_string(EH_NAME);
        amqpvalue_set_map_value(properties_map, name_key, name_value);
        //application_properties application_properties = amqpvalue_create_application_properties(properties_map);
        message_set_application_properties(message, properties_map);

        amqp_management_execute_operation_async(amqp_management, "READ", "com.microsoft:eventhub", NULL, message, on_read_operation_complete, amqp_management);

        bool keep_running = true;
        while (keep_running)
        {
            size_t current_memory_used;
            size_t maximum_memory_used;
            connection_dowork(connection);

            current_memory_used = gballoc_getCurrentMemoryUsed();
            maximum_memory_used = gballoc_getMaximumMemoryUsed();

            if (current_memory_used != last_memory_used)
            {
                (void)printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
                last_memory_used = current_memory_used;
            }
        }

        result = 0;

        amqp_management_destroy(amqp_management);
        session_destroy(session);
        connection_destroy(connection);
        platform_deinit();

        (void)printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        (void)printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();
    }

    return result;
}
