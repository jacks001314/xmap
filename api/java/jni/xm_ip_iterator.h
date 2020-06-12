/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_xmap_api_XMapIPIterator */

#ifndef _Included_com_xmap_api_XMapIPIterator
#define _Included_com_xmap_api_XMapIPIterator
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    iterate
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapIPIterator_iterate__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *, jobject, jstring, jstring, jstring, jint);

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    iterate
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapIPIterator_iterate__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jobject, jstring, jstring, jstring, jstring);

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    prepareIterate
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapIPIterator_prepareIterate
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    getNextIP
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_xmap_api_XMapIPIterator_getNextIP
  (JNIEnv *, jobject);

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    closeIterate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_xmap_api_XMapIPIterator_closeIterate
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
