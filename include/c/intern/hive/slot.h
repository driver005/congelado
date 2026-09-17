#ifndef TENSORFLOW_C_TF_HIVE_SLOT_H_
#define TENSORFLOW_C_TF_HIVE_SLOT_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFHiveSlot
    {
        void* plugin_data;
    } TFHiveSlot;

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_TF_HIVE_SLOT_H_
