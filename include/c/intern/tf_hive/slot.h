#ifndef TENSORFLOW_C_TF_HIVE_SLOT_H_
#define TENSORFLOW_C_TF_HIVE_SLOT_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Hive_Slot
    {
        void* plugin_data;
    } TF_Hive_Slot;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_TF_HIVE_SLOT_H_
