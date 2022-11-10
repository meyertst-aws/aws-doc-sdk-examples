/*
   Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
   SPDX-License-Identifier: Apache-2.0
*/

#include <gtest/gtest.h>
#include <fstream>
#include "awsdoc/s3/s3_examples.h"
#include "S3_GTests.h"

static const int BUCKETS_NEEDED = 1;

namespace AwsDocTest {
    // NOLINTNEXTLINE(readability-named-parameter)
    TEST_F(S3_GTests, put_bucket_acl) {
        std::vector<Aws::String> bucketNames = GetCachedS3Buckets(BUCKETS_NEEDED);
        ASSERT_GE(bucketNames.size(), BUCKETS_NEEDED) << "Failed to meet precondition" << std::endl;

        Aws::String canonicalUserID = GetCanonicalUserID();
        ASSERT_TRUE(!canonicalUserID.empty()) << "Failed to meet precondition" << std::endl;

        bool result = AwsDoc::S3::PutBucketAcl(bucketNames[0], canonicalUserID, "READ", "Canonical user",
                                               canonicalUserID, *s_clientConfig);

        ASSERT_TRUE(result);
    }
} // namespace AwsDocTest
