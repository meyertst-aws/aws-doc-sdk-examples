/*
   Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
   SPDX-License-Identifier: Apache-2.0
*/

#pragma once
#ifndef MEDICAL_IMAGING_EXAMPLES_MEDICAL_IMAGING_SAMPLES_H
#define MEDICAL_IMAGING_EXAMPLES_MEDICAL_IMAGING_SAMPLES_H

namespace AwsDoc {
    namespace Medical_Imaging {
        //! Routine which downloads a HealthImaging image frame.
        /*!
          \param dataStoreID The HealthImaging data store ID.
          \param imageSetID: The image set ID.
          \param frameID: The image frame ID.
          \param clientConfig: Aws client configuration.
        */

        bool GetImageFrame(const Aws::String &dataStoreID,
                                                    const Aws::String &imageSetID,
                                                    const Aws::String &frameID,
                                                    const Aws::Client::ClientConfiguration &clientConfig);

       } // Medical_Imaging
} // AwsDoc


#endif //MEDICAL_IMAGING_EXAMPLES_MEDICAL_IMAGING_SAMPLES_H
