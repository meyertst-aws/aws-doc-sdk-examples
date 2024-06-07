// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <aws/s3/model/Object.h>
#include <awsdoc/s3/s3_examples.h>

/**
 * Before running this C++ code example, set up your development environment, including your credentials.
 *
 * For more information, see the following documentation topic:
 *
 * https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/getting-started.html
 *
 * Purpose
 *
 * Demonstrates using the AWS SDK for C++ to list the objects in an S3 bucket.
 *
 */

//! Routine which demonstrates listing the objects in an S3 bucket.
/*!
  \fn ListObjects()
  \param bucketName Name of the S3 bucket.
  \param clientConfig Aws client configuration.
 */

// snippet-start:[s3.cpp.list_objects.code]
bool AwsDoc::S3::ListObjects(const Aws::String &bucketName,
                             Aws::Vector<Aws::String> &objectsResult,
                             const Aws::Client::ClientConfiguration &clientConfig) {
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::String continuationToken; // Used for paginated results;
    do {
        Aws::S3::Model::ListObjectsV2Request request;
        request.WithBucket(bucketName);

        if (!continuationToken.empty()) {
            request.SetContinuationToken(continuationToken);
        }

        Aws::S3::Model::ListObjectsV2Outcome outcome = s3_client.ListObjectsV2(request);

        if (!outcome.IsSuccess()) {
            std::cerr << "Error: ListObjects: " <<
                      outcome.GetError().GetMessage() << std::endl;
            return false;
        }
        else {
            const Aws::Vector<Aws::S3::Model::Object> &objects =
                    outcome.GetResult().GetContents();

            for (const Aws::S3::Model::Object &object: objects) {
                objectsResult.push_back(object.GetKey());
            }

            continuationToken = outcome.GetResult().GetNextContinuationToken();
        }

    } while (!continuationToken.empty());

    return true;
}
// snippet-end:[s3.cpp.list_objects.code]

/*
 *
 *   main function
 *
 * Prerequisites: Create a bucket containing at least one object.
 *
 * TODO(user): items: Set the following variables.
 * - bucketName: The name of the bucket containing the objects.
 *
 */

#ifndef EXCLUDE_MAIN_FUNCTION

int main()
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        //TODO(user): Name of a bucket in your account.
        //The bucket must have at least one object in it.  One way to achieve
        //this is to configure and run put_object.cpp's executable first.
        const Aws::String bucket_name = "<enter_bucket_name>";

        Aws::Client::ClientConfiguration clientConfig;
        // Optional: Set to the AWS Region in which the bucket was created (overrides config file).
        // clientConfig.region = "us-east-1";

        Aws::Vector <Aws::String> objectsResult;
        if (AwsDoc::S3::ListObjects(bucket_name, objectsResult, clientConfig)) {
            std::cout << objectsResult.size() << " object(s) retrieved." << std::endl;
            for (auto object : objectsResult) {
                std::cout << "   " << object << std::endl;
            }
        }

     }
    Aws::ShutdownAPI(options);

    return 0;
}

#endif  // EXCLUDE_MAIN_FUNCTION

