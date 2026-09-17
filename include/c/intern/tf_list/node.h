#ifndef TENSORFLOW_C_TF_LIST_NODE_H_
#define TENSORFLOW_C_TF_LIST_NODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFListNode
    {
        void* plugin_data;
    } TFListNode;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_TF_LIST_NODE_H_
