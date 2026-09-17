#ifndef TENSORFLOW_C_TF_FORWARD_LIST_NODE_H_
#define TENSORFLOW_C_TF_FORWARD_LIST_NODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_ForwardList_Node
    {
        void* plugin_data;
    } TF_ForwardList_Node;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_TF_FORWARD_LIST_NODE_H_
