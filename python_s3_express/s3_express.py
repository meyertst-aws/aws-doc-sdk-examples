# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import logging
import uuid
import time
from boto3 import client
from botocore.exceptions import ClientError, ParamValidationError


import cProfile, pstats

class TestS3Express:
    def __init__(
        self,
        region : str,
            availability_zone : str
    ):
        self.directory_bucket_name = None
        self.s3_express_client = None
        self.s3_regular_client = None
        self.region = region
        self.availability_zone = availability_zone


    def test_s3_express(self):
        self.s3_regular_client = client(service_name = "s3", region_name = self.region,)
        self.s3_express_client = client(service_name="s3", region_name=self.region,)

        bucket_prefix = 's3-express'

        # Construct the parts of a directory bucket name that is made unique with a UUID string.
        directory_bucket_suffix = f"--{self.availability_zone}--x-s3"
        max_uuid_length = 63 - len(bucket_prefix) - len(directory_bucket_suffix) - 1
        bucket_uuid = str(uuid.uuid4()).replace('-', '')[:max_uuid_length]

        directory_bucket_name = f"{bucket_prefix}-{bucket_uuid}{directory_bucket_suffix}"
        regular_bucket_name = f"{bucket_prefix}-regular-{bucket_uuid}"
        configuration = { 'Bucket': { 'Type' : 'Directory',
                                      'DataRedundancy' : 'SingleAvailabilityZone'},
                          'Location' : { 'Name' : self.availability_zone,
                                         'Type' :  'AvailabilityZone'}
                          }

        self.create_bucket(self.s3_express_client, directory_bucket_name, configuration)
        print(f"Created directory bucket, '{directory_bucket_name}'")
        self.directory_bucket_name = directory_bucket_name
        self.create_bucket(self.s3_regular_client, regular_bucket_name)
        print(f"Created regular bucket, '{regular_bucket_name}'")
        self.regular_bucket_name = regular_bucket_name

        bucket_object = "basic-text-object_key"
        TestS3Express.put_object(self.s3_regular_client, self.regular_bucket_name, bucket_object, "Some bucket text.")
        self.create_express_session()
        TestS3Express.put_object(self.s3_express_client, self.directory_bucket_name, bucket_object, "Some bucket text.")


        downloads = 200
        print(f"The number of downloads of the same object_key for this example is set at {downloads}.")

        print("Downloading from the Directory bucket.")
        enable_profile = True
        express_profiler = None
        if enable_profile:
            express_profiler = cProfile.Profile()
            express_profiler.enable()

        directory_time_start = time.time_ns()

        for index in range(downloads):
            TestS3Express.get_object(self.s3_express_client, self.directory_bucket_name, bucket_object)

        directory_time_difference = time.time_ns() - directory_time_start
        if express_profiler is not None :
            express_profiler.disable()
            stats = pstats.Stats(express_profiler).sort_stats('tottime')
            stats.dump_stats("express_stats.pstat")

        print("Downloading from the normal bucket.")

        regular_profiler = None
        if enable_profile:
            regular_profiler = cProfile.Profile()
            regular_profiler.enable()
        normal_time_start = time.time_ns()
        for index in range(downloads):
            TestS3Express.get_object(self.s3_regular_client, self.regular_bucket_name, bucket_object)

        normal_time_difference = time.time_ns() - normal_time_start

        if regular_profiler is not None:
            regular_profiler.disable()
            stats = pstats.Stats(regular_profiler).sort_stats('tottime')
            stats.dump_stats("regular_stats.psat")
        print(
            f"The directory bucket took {directory_time_difference} nanoseconds, while the normal bucket took {normal_time_difference}."
        )
        print(f"That's a difference of {normal_time_difference - directory_time_difference} nanoseconds, or")
        print(f"{(normal_time_difference - directory_time_difference) / 1000000000} seconds.")

        self.cleanup()

    def cleanup(self):

        if self.directory_bucket_name is not None:
            self.delete_bucket_and_objects(self.s3_express_client, self.directory_bucket_name)
            print(f"Deleted directory bucket, '{self.directory_bucket_name}'")
            self.directory_bucket_name = None

        if self.regular_bucket_name  is not None:
            self.delete_bucket_and_objects(self.s3_regular_client, self.regular_bucket_name)
            print(f"Deleted regular bucket, '{self.regular_bucket_name}'")
            self.regular_bucket_name = None

    @staticmethod
    def create_bucket(s3_client : client, bucket_name: str, bucket_configuration : dict[str, any] = None) -> None:
        """
        Creates a bucket.
        :param s3_client: The S3 client to use.
        :param bucket_name: The name of the bucket.
        :param bucket_configuration: The optional configuration for the bucket.
        """
        try:
            params = {"Bucket": bucket_name}
            if bucket_configuration:
                params["CreateBucketConfiguration"] = bucket_configuration

            s3_client.create_bucket(**params)
        except ClientError as client_error:
            logging.error(
                "Couldn't create the bucket %s. Here's why: %s",
                bucket_name,
                client_error.response["Error"]["Message"]
            )
            raise

    @staticmethod
    def delete_bucket_and_objects(s3_client : client, bucket_name: str) -> None:
        """
        Deletes a bucket and its objects.
        :param s3_client: The S3 client to use.
        :param bucket_name: The name of the bucket.
        """
        try:
            # Delete the objects in the bucket first. This is required for a bucket to be deleted.
            paginator = s3_client.get_paginator('list_objects_v2')
            page_iterator = paginator.paginate(Bucket=bucket_name)
            for page in page_iterator:
                if 'Contents' in page:
                    delete_keys = {'Objects': [{'Key': obj['Key']} for obj in page['Contents']]}
                    response = s3_client.delete_objects(Bucket=bucket_name, Delete=delete_keys)
                    if 'Errors' in response:
                        for error in response['Errors']:
                            logging.error("Couldn't delete object %s. Here's why: %s", error['Key'], error['Message'])


            s3_client.delete_bucket(Bucket=bucket_name)
        except ClientError as client_error:
            logging.error(
                "Couldn't delete the bucket %s. Here's why: %s",
                bucket_name,
                client_error.response["Error"]["Message"]
            )

    @staticmethod
    def put_object(s3_client : client, bucket_name: str, object_key: str, content: str) -> None:
        """
        Puts an object into a bucket.
        :param s3_client: The S3 client to use.
        :param bucket_name: The name of the bucket.
        :param object_key: The key of the object.
        :param content: The content of the object.
        """
        try:
            s3_client.put_object(Body=content, Bucket=bucket_name, Key=object_key)
        except ClientError as client_error:
            logging.error(
                "Couldn't put the object %s into bucket %s. Here's why: %s",
                object_key,
                bucket_name,
                client_error.response["Error"]["Message"]
            )
            raise


    @staticmethod
    def get_object(s3_client : client, bucket_name: str, object_key: str) -> None:
        """
        Gets an object from a bucket.
        :param s3_client: The S3 client to use.
        :param bucket_name: The name of the bucket.
        :param object_key: The key of the object.
        """
        try:
            s3_client.get_object(Bucket=bucket_name, Key=object_key)
        except ClientError as client_error:
            logging.error(
                "Couldn't get the object %s from bucket %s. Here's why: %s",
                object_key,
                bucket_name,
                client_error.response["Error"]["Message"]
            )
            raise

    def create_express_session(self) -> None:
        """
        Creates an express session.
        """
        try:
            self.s3_express_client.create_session(Bucket=self.directory_bucket_name)
        except ClientError as client_error:
            logging.error(
                "Couldn't create the express session for bucket %s. Here's why: %s",
                self.directory_bucket_name,
                client_error.response["Error"]["Message"],
            )
            raise


if __name__ == "__main__":
    test_s3_express = None
    try:
        test_s3_express = TestS3Express(region ="us-east-1",
                                        availability_zone= "use1-az4")
        test_s3_express.test_s3_express()
    except ClientError as err:
        logging.exception("Something went wrong with the demo!")
        if test_s3_express is not None:
            test_s3_express.cleanup()
    except ParamValidationError as err:
        logging.exception("Parameter validation error in demo!")
        if test_s3_express is not None:
            test_s3_express.cleanup()
    except TypeError as err:
        logging.exception("Type error in demo!")
        if test_s3_express is not None:
            test_s3_express.cleanup()

