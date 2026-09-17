#ifndef TENSORFLOW_C_TF_FORWARD_LIST_NODE_H_
#define TENSORFLOW_C_TF_FORWARD_LIST_NODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFForwardListNode
    {
        void* plugin_data;
    } TFForwardListNode;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_TF_FORWARD_LIST_NODE_H_
