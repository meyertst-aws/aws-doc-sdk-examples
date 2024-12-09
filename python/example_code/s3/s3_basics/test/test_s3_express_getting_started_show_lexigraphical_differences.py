# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Unit tests for demo_bucket_basics.py.
"""

import pytest

import boto3
from botocore.exceptions import ClientError
from botocore import waiter
import os
import sys
import uuid

script_dir = os.path.dirname(os.path.abspath(__file__))

# Append directory for s3_express_getting_started.py
sys.path.append(os.path.join(script_dir, ".."))
import s3_express_getting_started


@pytest.mark.parametrize(
    "error_code, stop_on_index",
    [
        ("TESTERROR-stub_put_object_other_object", 0),
        ("TESTERROR-stub_put_object_other_object", 1),
        ("TESTERROR-stub_put_object_alt_object", 2),
        ("TESTERROR-stub_put_object_alt_object", 3),
        ("TESTERROR-stub_put_object_other_alt_object", 4),
        ("TESTERROR-stub_put_object_other_alt_object", 5),
        ("TESTERROR-stub_list_objects_directory", 6),
        ("TESTERROR-stub_list_objects_regular", 7),
    ]
)
def test_s3_express_scenario(
    make_stubber, stub_runner, error_code   , stop_on_index, monkeypatch
):
    region = "us-east-1"
    cloud_formation_resource = boto3.resource("cloudformation")
    cloud_formation_stubber = make_stubber(cloud_formation_resource.meta.client)

    ec2_client = boto3.client("ec2", region_name=region)
    ec2_stubber = make_stubber(ec2_client)

    iam_client = boto3.client("iam")
    iam_stubber = make_stubber(iam_client)

    s3_client = boto3.client("s3")
    s3_stubber = make_stubber(s3_client)

    vpc_id = "XXXXXXXXXXXXXXXXXXXXX"
    route_table_id = "XXXXXXXXXXXXXXXXXXXXX"

    filter = [{"Name": "vpc-id", "Values": [vpc_id]}]

    service_name = f"com.amazonaws.{region}.s3express"
    my_uuid = "0000"

    stack_name = f"cfn-stack-s3-express-basics--{my_uuid}"
    template = s3_express_getting_started.S3ExpressScenario.get_template_as_string()
    stack_id  = f"arn:aws:cloudformation:{region}:000000000000:stack/{stack_name}"

    express_user_name = "s3-express-user"
    regular_user_name = "regular-user"

    outputs = [
        {"OutputKey": "RegularUser", "OutputValue": regular_user_name},
        {"OutputKey": "ExpressUser", "OutputValue": express_user_name},
    ]

    availability_zone_filter = [{"Name": "region-name", "Values": [region]}]
    availability_zone_ids = ["use1-az2"]

    bucket_name_prefix = "amzn-s3-demo-bucket"
    directory_bucket_name = f"{bucket_name_prefix}-{my_uuid}--{availability_zone_ids[0]}--x-s3"
    regular_bucket_name = f"{bucket_name_prefix}-regular-{my_uuid}"

    directory_bucket_configuration = { 'Bucket': { 'Type' : 'Directory',
                                      'DataRedundancy' : 'SingleAvailabilityZone'},
                          'Location' : { 'Name' : availability_zone_ids[0],
                                         'Type' :  'AvailabilityZone'}
                          }

    object_name = "basic-text-object"
    other_object = f"other/{object_name}"
    alt_object = f"alt/{object_name}"
    other_alt_object = f"other/alt/{object_name}"

    object_keys = [object_name, other_object, alt_object, other_alt_object]

    inputs = [
    ]

    monkeypatch.setattr("builtins.input", lambda x: inputs.pop(0))

    s3_express_getting_started.use_press_enter_to_continue = False


    with stub_runner(error_code, stop_on_index) as runner:
        runner.add (s3_stubber.stub_put_object, regular_bucket_name, other_object, "")
        runner.add (s3_stubber.stub_put_object, directory_bucket_name, other_object, "")
        runner.add (s3_stubber.stub_put_object, regular_bucket_name, alt_object, "")
        runner.add (s3_stubber.stub_put_object, directory_bucket_name, alt_object, "")
        runner.add (s3_stubber.stub_put_object, regular_bucket_name, other_alt_object, "")
        runner.add (s3_stubber.stub_put_object, directory_bucket_name, other_alt_object, "")

        runner.add(s3_stubber.stub_list_objects_v2, directory_bucket_name, object_keys)
        runner.add(s3_stubber.stub_list_objects_v2, regular_bucket_name, object_keys)


    def mock_wait(self, **kwargs):
        return

    # Mock the waiters.
    monkeypatch.setattr(waiter.Waiter, "wait", mock_wait)

    scenario = s3_express_getting_started.S3ExpressScenario(cloud_formation_resource, ec2_client,
                                                            iam_client)
    scenario.s3_express_client = s3_client
    scenario.s3_regular_client = s3_client
    scenario.regular_bucket_name = regular_bucket_name
    scenario.directory_bucket_name = directory_bucket_name

    def mock_create_s3_client(self, **kwargs):
        return s3_client

    monkeypatch.setattr(scenario, "create_s3__client_with_access_key_credentials", mock_create_s3_client)

    monkeypatch.setattr(uuid, "uuid4", lambda : my_uuid)

    # Stubbing the client functions in 'delete_bucket_and_objects' does not work.
    # This may be caused by the depth of the call stack.
    # delete_bucket_and_objects is tested separately.
    monkeypatch.setattr(scenario, "delete_bucket_and_objects", lambda arg1, arg2: None)

    monkeypatch.setattr(scenario, "tear_done_vpc", lambda arg2: None)

    if error_code is None:
        scenario.show_lexigraphical_differences(object_name)
    else:
        with pytest.raises(ClientError) as exc_info:
            scenario.show_lexigraphical_differences(object_name)
        assert exc_info.value.response["Error"]["Code"] == error_code
