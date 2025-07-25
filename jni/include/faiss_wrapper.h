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

#ifndef OPENSEARCH_KNN_FAISS_WRAPPER_H
#define OPENSEARCH_KNN_FAISS_WRAPPER_H

#include "jni_util.h"
#include "faiss_index_service.h"
#include "faiss_stream_support.h"
#include <jni.h>

namespace knn_jni {
    namespace faiss_wrapper {
        jlong InitIndex(knn_jni::JNIUtilInterface *jniUtil, JNIEnv *env, jlong numDocs, jint dimJ, jobject parametersJ, IndexService *indexService);

        void InsertToIndex(knn_jni::JNIUtilInterface *jniUtil, JNIEnv *env, jintArray idsJ, jlong vectorsAddressJ, jint dimJ, jlong indexAddr, jint threadCount, IndexService *indexService);

        void WriteIndex(knn_jni::JNIUtilInterface *jniUtil, JNIEnv *env, jobject output, jlong indexAddr, IndexService *indexService);

        // Create an index with ids and vectors. Instead of creating a new index, this function creates the index
        // based off of the template index passed in. The index is serialized to indexPathJ.
        void CreateIndexFromTemplate(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jintArray idsJ,
                                     jlong vectorsAddressJ, jint dimJ, jobject output, jbyteArray templateIndexJ,
                                     jobject parametersJ);

        // Create an index with ids and vectors. Instead of creating a new index, this function creates the index
        // based off of the template index passed in. The index is serialized to indexPathJ.
        void CreateBinaryIndexFromTemplate(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jintArray idsJ,
                                           jlong vectorsAddressJ, jint dimJ, jobject output, jbyteArray templateIndexJ,
                                           jobject parametersJ);

        // Create a index with ids and byte vectors. Instead of creating a new index, this function creates the index
        // based off of the template index passed in. The index is serialized to indexPathJ.
        void CreateByteIndexFromTemplate(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jintArray idsJ,
                                         jlong vectorsAddressJ, jint dimJ, jobject output, jbyteArray templateIndexJ,
                                         jobject parametersJ);

        // Load an index from indexPathJ into memory.
        //
        // Return a pointer to the loaded index
        jlong LoadIndex(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jstring indexPathJ);

        // Loads an index with a reader implemented IOReader
        //
        // Returns a pointer of the loaded index
        jlong LoadIndexWithStream(faiss::IOReader* ioReader);

        // Loads an index with a reader implemented IOReader. The index
        // is expected to be a binary index. For ADC, it will be converted into a
        // float index.
        //
        // Returns a pointer of the loaded index
        jlong LoadIndexWithStreamADCParams(faiss::IOReader* ioReader, knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jobject methodParamsJ);

        // Loads a one-bit ADC index from the binary index read with IOReader. The index is altered to support scoring full precision query vectors
        // against document codes.
        //
        // Returns a pointer of the loaded index
        jlong LoadIndexWithStreamADC(faiss::IOReader* ioReader, faiss::MetricType metricType);

        // Load a binary index from indexPathJ into memory.
        //
        // Return a pointer to the loaded index
        jlong LoadBinaryIndex(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jstring indexPathJ);

        // Loads a binary index with a reader implemented IOReader
        //
        // Returns a pointer of the loaded index
        jlong LoadBinaryIndexWithStream(faiss::IOReader* ioReader);

        // Check if a loaded index requires shared state
        bool IsSharedIndexStateRequired(jlong indexPointerJ);

        // Initializes the shared index state from an index. Note, this will not set the state for
        // the index pointed to by indexPointerJ. To set it, SetSharedIndexState needs to be called.
        //
        // Return a pointer to the shared index state
        jlong InitSharedIndexState(jlong indexPointerJ);

        // Sets the sharedIndexState for an index
        void SetSharedIndexState(jlong indexPointerJ, jlong shareIndexStatePointerJ);

        /**
         *  Execute a query against the index located in memory at indexPointerJ
         *
         * Parameters:
         * methodParamsJ: introduces a map to have additional method parameters
         *
         * Return an array of KNNQueryResults
         */
        jobjectArray QueryIndex(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jlong indexPointerJ,
                                jfloatArray queryVectorJ, jint kJ, jobject methodParamsJ, jintArray parentIdsJ);

        /**
         *  Execute a query against the index located in memory at indexPointerJ along with Filters
         *
         * Parameters:
         * methodParamsJ: introduces a map to have additional method parameters
         *
         * Return an array of KNNQueryResults
        */
        jobjectArray QueryIndex_WithFilter(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jlong indexPointerJ,
                                           jfloatArray queryVectorJ, jint kJ, jobject methodParamsJ, jlongArray filterIdsJ,
                                           jint filterIdsTypeJ, jintArray parentIdsJ);

        // Execute a query against the binary index located in memory at indexPointerJ along with Filters
        //
        // Return an array of KNNQueryResults
        jobjectArray QueryBinaryIndex_WithFilter(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jlong indexPointerJ,
                                                 jbyteArray queryVectorJ, jint kJ, jobject methodParamsJ, jlongArray filterIdsJ, jint filterIdsTypeJ, jintArray parentIdsJ);

        // Free the index located in memory at indexPointerJ
        void Free(jlong indexPointer, jboolean isBinaryIndexJ);

        // Free shared index state in memory at shareIndexStatePointerJ
        void FreeSharedIndexState(jlong shareIndexStatePointerJ);

        // Perform initilization operations for the library
        void InitLibrary();

        // Create an empty index defined by the values in the Java map, parametersJ. Train the index with
        // the vector of floats located at trainVectorsPointerJ.
        //
        // Return the serialized representation
        jbyteArray TrainIndex(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jobject parametersJ, jint dimension,
                              jlong trainVectorsPointerJ);

        // Create an empty binary index defined by the values in the Java map, parametersJ. Train the index with
        // the vector of floats located at trainVectorsPointerJ.
        //
        // Return the serialized representation
        jbyteArray TrainBinaryIndex(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jobject parametersJ, jint dimension,
                                    jlong trainVectorsPointerJ);

        // Create an empty byte index defined by the values in the Java map, parametersJ. Train the index with
        // the byte vectors located at trainVectorsPointerJ.
        //
        // Return the serialized representation
        jbyteArray TrainByteIndex(knn_jni::JNIUtilInterface * jniUtil, JNIEnv * env, jobject parametersJ, jint dimension,
                                  jlong trainVectorsPointerJ);

        /*
         * Perform a range search with filter against the index located in memory at indexPointerJ.
         *
         * @param indexPointerJ - pointer to the index
         * @param queryVectorJ - the query vector
         * @param radiusJ - the radius for the range search
         * @param methodParamsJ - the method parameters
         * @param maxResultsWindowJ - the maximum number of results to return
         * @param filterIdsJ - the filter ids
         * @param filterIdsTypeJ - the filter ids type
         * @param parentIdsJ - the parent ids
         *
         * @return an array of RangeQueryResults
         */
        jobjectArray RangeSearchWithFilter(knn_jni::JNIUtilInterface *jniUtil, JNIEnv *env, jlong indexPointerJ, jfloatArray queryVectorJ,
                                           jfloat radiusJ, jobject methodParamsJ, jint maxResultWindowJ, jlongArray filterIdsJ, jint filterIdsTypeJ, jintArray parentIdsJ);

        /*
         * Perform a range search against the index located in memory at indexPointerJ.
         *
         * @param indexPointerJ - pointer to the index
         * @param queryVectorJ - the query vector
         * @param radiusJ - the radius for the range search
         * @param methodParamsJ - the method parameters
         * @param maxResultsWindowJ - the maximum number of results to return
         * @param parentIdsJ - the parent ids
         *
         * @return an array of RangeQueryResults
         */
        jobjectArray RangeSearch(knn_jni::JNIUtilInterface *jniUtil, JNIEnv *env, jlong indexPointerJ, jfloatArray queryVectorJ,
                                 jfloat radiusJ, jobject methodParamsJ, jint maxResultWindowJ, jintArray parentIdsJ);

        /**
         * Translates a space type string to a Faiss metric type
         *
         * @param spaceType The k-NN space type string to translate
         * @return The corresponding Faiss metric type
         * @throws std::runtime_error if the space type is invalid
         */
        faiss::MetricType TranslateSpaceToMetric(const std::string& spaceType);
    }
}

#endif //OPENSEARCH_KNN_FAISS_WRAPPER_H
