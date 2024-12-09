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
        ("TESTERROR-stub_list_objects_directory", 0),
        ("TESTERROR-stub_delete_objects_directory", 1),
        ("TESTERROR-stub_delete_bucket_directory", 2),
    ]
)
def test_s3_express_scenario_delete_bucket_and_objects(
    make_stubber, stub_runner, error_code  , stop_on_index, monkeypatch
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

    my_uuid = "0000"

    availability_zone_ids = ["use1-az2"]

    bucket_name_prefix = "amzn-s3-demo-bucket"
    directory_bucket_name = f"{bucket_name_prefix}-{my_uuid}--{availability_zone_ids[0]}--x-s3"
    regular_bucket_name = f"{bucket_name_prefix}-regular-{my_uuid}"
    object_name = "basic-text-object"
    other_object = f"other/{object_name}"
    alt_object = f"alt/{object_name}"
    other_alt_object = f"other/alt/{object_name}"

    object_keys = [object_name, other_object, alt_object, other_alt_object]

    with stub_runner(error_code, stop_on_index) as runner:
        runner.add(s3_stubber.stub_list_objects_v2, directory_bucket_name, object_keys)
        runner.add(s3_stubber.stub_delete_objects, directory_bucket_name, object_keys)
        runner.add(s3_stubber.stub_delete_bucket, directory_bucket_name)

    scenario = s3_express_getting_started.S3ExpressScenario(cloud_formation_resource, ec2_client,
                                                            iam_client)

    scenario.s3_express_client = s3_client
    scenario.s3_regular_client = s3_client
    scenario.regular_bucket_name = regular_bucket_name
    scenario.directory_bucket_name = directory_bucket_name

    s3_express_getting_started.S3ExpressScenario.delete_bucket_and_objects(s3_client, directory_bucket_name)

@pytest.mark.parametrize(
    "error_code, stop_on_index",
    [
        ("TESTERROR-stub_delete_vpc", 1),
    ]
)
def test_s3_express_scenario_tear_done_vpc(
    make_stubber, stub_runner, error_code  , stop_on_index, monkeypatch
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

    my_uuid = "0000"

    availability_zone_ids = ["use1-az2"]

    bucket_name_prefix = "amzn-s3-demo-bucket"
    directory_bucket_name = f"{bucket_name_prefix}-{my_uuid}--{availability_zone_ids[0]}--x-s3"
    regular_bucket_name = f"{bucket_name_prefix}-regular-{my_uuid}"
    object_name = "basic-text-object"
    other_object = f"other/{object_name}"
    alt_object = f"alt/{object_name}"
    other_alt_object = f"other/alt/{object_name}"
    vpc_id = "XXXXXXXXXXXXXXXXXXXXX"
    vpc_endpoint_id = f"vpce-{vpc_id}"

    with stub_runner(error_code, stop_on_index) as runner:
        runner.add(ec2_stubber.stub_delete_vpc_endpoints, [vpc_endpoint_id])
        runner.add(ec2_stubber.stub_delete_vpc, vpc_id)

    scenario = s3_express_getting_started.S3ExpressScenario(cloud_formation_resource, ec2_client,
                                                            iam_client)

    scenario.vpc_id = vpc_id
    scenario.vcp_endpoint_id = vpc_endpoint_id

    scenario.tear_done_vpc()

