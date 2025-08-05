/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * The OpenSearch Contributors require contributions made to
 * this file be licensed under the Apache-2.0 license or a
 * compatible open source license.
 *
 * Modifications Copyright OpenSearch Contributors. See
 * GitHub history for details.
 */

#ifndef OPENSEARCH_KNN_JNI_FAISS_STREAM_SUPPORT_H
#define OPENSEARCH_KNN_JNI_FAISS_STREAM_SUPPORT_H

#include "faiss/impl/io.h"
#include "jni_util.h"
#include "native_engines_stream_support.h"
#include "parameter_utils.h"

#include <jni.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

namespace knn_jni {
namespace stream {



/**
 * A glue component inheriting IOReader to be passed down to Faiss library.
 * This will then indirectly call the mediator component and eventually read required bytes from Lucene's IndexInput.
 */
class FaissOpenSearchIOReader final : public faiss::IOReader {
public:
    explicit FaissOpenSearchIOReader(NativeEngineIndexInputMediator* _mediator)
        : faiss::IOReader(),
          mediator(knn_jni::util::ParameterCheck::require_non_null(_mediator, "mediator")),
          cachedEnv(_mediator->getEnv()) {

        name = "FaissOpenSearchIOReader";
    }


    ~FaissOpenSearchIOReader() override {
        if (vectorReaderGlobalRef && cachedEnv) {
            cachedEnv->DeleteGlobalRef(vectorReaderGlobalRef);
        }
    }

    size_t operator()(void* ptr, size_t size, size_t nitems) override {
        const auto bytes = size * nitems;
        mediator->copyBytes(bytes, static_cast<uint8_t*>(ptr));
        return nitems;
    }

    bool copy(void* dest, int expectedByteSize, bool isFloat) override {
        JNIEnv* env = cachedEnv;

        jobject readStream = mediator->getJavaObject();

        if (!vectorReaderGlobalRef) {
            jclass streamClass = env->GetObjectClass(readStream);
            jmethodID getVectorsMid = env->GetMethodID(
                streamClass,
                "getFullPrecisionVectors",
                "()Lorg/opensearch/knn/index/store/VectorReader;"
            );
            if (!getVectorsMid) {
                return false;
            }

            jobject vectorReader = env->CallObjectMethod(readStream, getVectorsMid);
            if (env->ExceptionCheck() || vectorReader == nullptr) {
                return false;
            }

            vectorReaderGlobalRef = env->NewGlobalRef(vectorReader);
            if (vectorReaderGlobalRef == nullptr) {
                return false;
            }
        }


        if (isFloat) {
            jmethodID nextFloatMid = getNextFloatVectorMethod(mediator, env);
            jfloatArray vector = (jfloatArray) env->CallObjectMethod(vectorReaderGlobalRef, nextFloatMid);
            if (env->ExceptionCheck() || vector == nullptr) {
                return false;
            }

            jsize length = env->GetArrayLength(vector);
            jfloat* elems = env->GetFloatArrayElements(vector, nullptr);

            JNIReleaseElements release_elems([=]() {
                env->ReleaseFloatArrayElements(vector, elems, JNI_ABORT);
            });

            int vectorByteSize = sizeof(float) * length;
            if (vectorByteSize != expectedByteSize) {
                return false;
            }

            std::memcpy(dest, elems, vectorByteSize);
            return true;

        }
        else {
            jmethodID nextByteMid = getNextByteVectorMethod(mediator, env);
            jbyteArray vector = (jbyteArray) env->CallObjectMethod(vectorReaderGlobalRef, nextByteMid);
            if (env->ExceptionCheck() || vector == nullptr) {
                return false;
            }

            jsize length = env->GetArrayLength(vector);
            jbyte* elems = env->GetByteArrayElements(vector, nullptr);

            JNIReleaseElements release_elems([=]() {
                env->ReleaseByteArrayElements(vector, elems, JNI_ABORT);
            });

            int vectorByteSize = sizeof(float) * length;
            if (vectorByteSize != expectedByteSize) {
                return false;
            }

            float* floatDest = static_cast<float*>(dest);
            for (int i = 0; i < length; ++i) {
                floatDest[i] = static_cast<float>(elems[i]);
            }

            return true;
        }
    }

private:
    static jclass getVectorReaderClass(NativeEngineIndexInputMediator* mediator, JNIEnv* env) {
        static jclass VECTOR_READER_CLASS =
            mediator->getJNIUtil()->FindClassFromJNIEnv(env, "org/opensearch/knn/index/store/VectorReader");
        return VECTOR_READER_CLASS;
    }

    static jmethodID getNextFloatVectorMethod(NativeEngineIndexInputMediator* mediator, JNIEnv* env) {
        static jmethodID MID =
            mediator->getJNIUtil()->GetMethodID(env, getVectorReaderClass(mediator, env), "nextFloatVector", "()[F");
        return MID;
    }

    static jmethodID getNextByteVectorMethod(NativeEngineIndexInputMediator* mediator, JNIEnv* env) {
        static jmethodID MID =
            mediator->getJNIUtil()->GetMethodID(env, getVectorReaderClass(mediator, env), "nextByteVector", "()[B");
        return MID;
    }

    NativeEngineIndexInputMediator* mediator;
    jobject vectorReaderGlobalRef = nullptr;
    JNIEnv* cachedEnv = nullptr;
}; // class FaissOpenSearchIOReader


/**
 * A glue component inheriting IOWriter to delegate IO processing down to the given
 * mediator. The mediator is expected to do write bytes via the provided Lucene's IndexOutput.
 */
class FaissOpenSearchIOWriter final : public faiss::IOWriter {
 public:
  explicit FaissOpenSearchIOWriter(NativeEngineIndexOutputMediator *_mediator)
      : faiss::IOWriter(),
        mediator(knn_jni::util::ParameterCheck::require_non_null(_mediator, "mediator")) {
    name = "FaissOpenSearchIOWriter";
  }

  size_t operator()(const void *ptr, size_t size, size_t nitems) final {
    const auto writeBytes = size * nitems;
    if (writeBytes > 0) {
      mediator->writeBytes(reinterpret_cast<const uint8_t *>(ptr), writeBytes);
    }
    return nitems;
  }

  // return a file number that can be memory-mapped
  int filedescriptor() final {
    throw std::runtime_error("filedescriptor() is not supported in FaissOpenSearchIOWriter.");
  }

  void flush() {
    mediator->flush();
  }

 private:
  NativeEngineIndexOutputMediator *mediator;
};  // class FaissOpenSearchIOWriter



}
}

#endif //OPENSEARCH_KNN_JNI_FAISS_STREAM_SUPPORT_H
