#ifndef TENSORFLOW_C_EXTERN_IO_CONNECTION_H_
#define TENSORFLOW_C_EXTERN_IO_CONNECTION_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFServerConnection
    {
        void* plugin_data;
    } TFServerConnection;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_EXTERN_IO_CONNECTION_H_
