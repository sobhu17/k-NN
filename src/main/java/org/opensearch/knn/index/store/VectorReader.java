/*
 * Copyright OpenSearch Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

package org.opensearch.knn.index.store;

import org.apache.lucene.index.FloatVectorValues;
import org.apache.lucene.index.KnnVectorValues.DocIndexIterator;
import java.io.IOException;

public class VectorReader {
    private final FloatVectorValues floatVectorValues;
    private final DocIndexIterator iterator;

    public VectorReader(FloatVectorValues floatVectorValues) {
        this.floatVectorValues = floatVectorValues;
        this.iterator = floatVectorValues.iterator();
    }

    public float[] next() throws IOException {
        int docId = iterator.nextDoc();
        System.out.println("Calling next(): docId = " + docId);
        if (docId != DocIndexIterator.NO_MORE_DOCS) {
            return floatVectorValues.vectorValue(docId);
        } else {
            return null;
        }
    }

//    public native float[] getNextVector();
}
