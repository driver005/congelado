#ifndef TENSORFLOW_C_EXTERN_IO_CONNECTION_H_
#define TENSORFLOW_C_EXTERN_IO_CONNECTION_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Server_Connection
    {
        void* plugin_data;
    } TF_Server_Connection;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_EXTERN_IO_CONNECTION_H_
