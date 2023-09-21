/*
Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
SPDX-License-Identifier: Apache-2.0
*/

#include <iostream>
#include <aws/core/Aws.h>
#include <fstream>
#include <aws/medical-imaging/MedicalImagingClient.h>
#include <aws/medical-imaging/model/GetImageFrameRequest.h>
#include "medical-imaging_samples.h"

// TODO: remove this
#include "openjph/ojph_arch.h"
#include "openjph/ojph_codestream.h"
#include "openjph/ojph_file.h"
#include "openjph/ojph_defs.h"
#include "openjph/ojph_params.h"
#include <openjph/ojph_mem.h>

/**
 * Before running this C++ code example, set up your development environment, including your credentials.
 *
 * For more information, see the following documentation topic:
 *
 * https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/getting-started.html
 *
 * Purpose
 *
 * Demonstrates using the AWS SDK for C++ to download an AWS HealthImaging image frame.
 *
 */

//! Routine which downloads a HealthImaging image frame.
/*!
  \param dataStoreID The HealthImaging data store ID.
  \param imageSetID: The image set ID.
  \param frameID: The image frame ID.
  \param clientConfig: Aws client configuration.
*/

// snippet-start:[cpp.example_code.medical-imaging.GetImageFrame]
bool AwsDoc::Medical_Imaging::GetImageFrame(const Aws::String &dataStoreID,
                           const Aws::String &imageSetID,
                           const Aws::String &frameID,
                           const Aws::Client::ClientConfiguration &clientConfig) {
    Aws::MedicalImaging::MedicalImagingClient client(clientConfig);

    Aws::MedicalImaging::Model::GetImageFrameRequest request;
    request.SetDatastoreId(dataStoreID);
    request.SetImageSetId(imageSetID);

    Aws::MedicalImaging::Model::ImageFrameInformation imageFrameInformation;
    imageFrameInformation.SetImageFrameId(frameID);
    request.SetImageFrameInformation(imageFrameInformation);

    Aws::MedicalImaging::Model::GetImageFrameOutcome outcome = client.GetImageFrame(request);

    if (outcome.IsSuccess()) {
        std::cout << "Successfully retrieved image frame." << std::endl;
        auto &buffer = outcome.GetResult().GetImageFrameBlob();

        const Aws::String filename = "frame.jph";
        {
            std::ofstream outfile(filename, std::ios::binary);
            outfile << buffer.rdbuf();
        }

        ojph::j2c_infile j2c_file;
        j2c_file.open(filename.c_str());
        ojph::codestream codestream;
        codestream.read_headers(&j2c_file);

        ojph::param_siz siz = codestream.access_siz();
        int width= siz.get_image_extent().x - siz.get_image_offset().x;
        int height = siz.get_image_extent().y - siz.get_image_offset().y;
        int componentCount = siz.get_num_components();
        int bitsPerSample = siz.get_bit_depth(0);
        int isSigned = siz.is_signed(0);

        std::cout << "Image size: " << width << "x" << height << std::endl;
        std::cout << "Component count: " << componentCount << std::endl;
        std::cout << "Bits per sample: " << bitsPerSample << std::endl;
        std::cout << "Is signed: " << isSigned << std::endl;

        ojph::param_cod cod = codestream.access_cod();
        int numDecompositions_ = cod.get_num_decompositions();
        bool isReversible_ = cod.is_reversible();
        int progressionOrder_ = cod.get_progression_order();
        int numLayers_ = cod.get_num_layers();
        bool isUsingColorTransform_ = cod.is_using_color_transform();

        std::cout << "Num decompositions: " << numDecompositions_ << std::endl;
        std::cout << "Is reversible: " << isReversible_ << std::endl;
        std::cout << "Progression order: " << progressionOrder_ << std::endl;
        std::cout << "Num layers: " << numLayers_ << std::endl;
        std::cout << "Is using color transform: " << isUsingColorTransform_ << std::endl;
        int32_t max = INT32_MIN;
        ojph::ui32 comp_num;
        codestream.create();
        for (int y = 0; y < height; y++) {
            ojph::line_buf *line = codestream.pull(comp_num);
            for (int x = 0; x < width; x++) {
                max = std::max(max, line->i32[x]);
            }
        }

        std::cout << "Max value: " << max << std::endl;
    }
    else {
        std::cout << "Error retrieving image frame." << outcome.GetError().GetMessage() << std::endl;

    }

    return outcome.IsSuccess();
}
// snippet-end:[cpp.example_code.medical-imaging.GetImageFrame]

/*
 *
 * main function
 *
 * Prerequisites: An image set in a HealthImaging data store.
 *
 *  Usage: 'Usage: run_get_image_frame <datastore_id> <image_set_id> <frame_id>'
 *
*/

#ifndef TESTING_BUILD

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Usage: run_get_image_frame <datastore_id> <image_set_id> <frame_id>" << std::endl;
        return 1;
    }
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::String dataStoreID = argv[1];
        Aws::String imageSetID = argv[2];
        Aws::String frameID = argv[3];

        Aws::Client::ClientConfiguration clientConfig;
        // Optional: Set to the AWS Region in which the bucket was created (overrides config file).
        // clientConfig.region = "us-east-1";

        AwsDoc::Medical_Imaging::GetImageFrame(dataStoreID, imageSetID, frameID, clientConfig);
    }
    Aws::ShutdownAPI(options);

    return 0;
}

#endif // TESTING_BUILD