# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Unit tests for prepare_application in iotsitewise_getting_started.py.
"""

import pytest
from botocore.exceptions import ClientError
from iotsitewise_getting_started import IoTSitewiseGettingStarted
import iotsitewise_getting_started
from botocore import waiter
import time

class MockManager:
    def __init__(self, stub_runner, scenario_data, input_mocker):
        self.scenario_data = scenario_data
        iotsitewise_getting_started.use_press_enter_to_continue = False
        self.stack_name = "python-iot-sitewise-basics"
        self.capabilities = ['CAPABILITY_NAMED_IAM']
        self.cfn_template =  IoTSitewiseGettingStarted.get_template_as_string()
        self.stack_id = "arn:aws:cloudformation:us-east-1:123456789012:stack/myteststack/466df9e0-0dff-08e3-8e2f-5088487c4896"
        self.outputs = [
            {
                'OutputKey': 'SitewiseRoleArn',
                'OutputValue': 'arn:aws:iam::123456789012:role/Test-Role'
            },
        ]

        self.temperature_property_name = "temperature"
        self.humidity_property_name = "humidity"
        self.asset_model_properties = properties = [
                {
                    "name": self.temperature_property_name,
                    "dataType": "DOUBLE",
                    "type": {
                        "measurement": {},
                    },
                },
                {
                    "name": self.humidity_property_name,
                    "dataType": "DOUBLE",
                    "type": {
                        "measurement": {},
                    },
                }
            ]
        self.asset_model_id = "a1b2c3d4-5678-90ab-cdef-11111EXAMPLE"
        self.asset_id = "a1b2c3d4-5678-90ab-cdef-33333EXAMPLE"
        self.humidity_property_id = "01234567-1234-0123-1234-0123456789ae"
        self.temperature_property_id = "12345678-1234-0123-1234-0123456789ae"
        self.values = [{"propertyId": self.humidity_property_id,
                   "valueType" : "doubleValue",
                   "value" : 65.0},
                  {"propertyId": self.temperature_property_id,
                   "valueType": "doubleValue",
                   "value": 23.5}]
        self.time_ns = time.time_ns()
        answers = []
        input_mocker.mock_answers(answers)
        self.stub_runner = stub_runner

    def setup_stubs(self, error, stop_on, iot_sitewise_stubber, cloud_formation_stubber, monkeypatch):
        with self.stub_runner(error, stop_on) as runner:
            runner.add(cloud_formation_stubber.stub_create_stack, self.stack_name, self.cfn_template,
                        self.capabilities, self.stack_id)
            runner.add(cloud_formation_stubber.stub_describe_stacks, self.stack_name, "CREATE_COMPLETE", self.outputs)
            runner.add(iot_sitewise_stubber.stub_create_asset_model, "MyAssetModel1", "This is a sample asset model description.", self.asset_model_properties,
                       self.asset_model_id)
            runner.add(iot_sitewise_stubber.stub_create_asset, "MyAsset1",
                       self.asset_model_id, self.asset_id)
            runner.add(
                iot_sitewise_stubber.stub_list_asset_model_properties,
                self.asset_model_id,
                self.humidity_property_name,
                self.humidity_property_id,
                "DOUBLE",
                truncated = True
            )
            runner.add(
                iot_sitewise_stubber.stub_list_asset_model_properties,
                self.asset_model_id,
                self.temperature_property_name,
                self.temperature_property_id,
                "DOUBLE",
                nextToken="test-token",
                raise_and_continue=True,
            )
            runner.add(iot_sitewise_stubber.stub_batch_put_asset_property_value, self.asset_id,
                       self.scenario_data.scenario.iot_sitewise_wrapper.entry_id,
            self.values)

        def mock_wait(self, **kwargs):
            return


        monkeypatch.setattr(waiter.Waiter, "wait", mock_wait)

        monkeypatch.setattr(time, "time_ns", lambda:self.time_ns )


@pytest.fixture
def mock_mgr(stub_runner, scenario_data, input_mocker):
    return MockManager(stub_runner, scenario_data, input_mocker)

@pytest.mark.integ
def test_iot_sitewise_getting_started(mock_mgr, capsys, monkeypatch):
    mock_mgr.setup_stubs(None, None, mock_mgr.scenario_data.iot_sitewise_stubber    ,
                          mock_mgr.scenario_data.cloud_formation_stubber,  monkeypatch)

    mock_mgr.scenario_data.scenario.run()



@pytest.mark.parametrize(
    "error, stop_on_index",
    [
        ("TESTERROR-stub_create_stack", 0),
        ("TESTERROR-stub_describe_stacks", 1),
        ("TESTERROR-stub_create_asset_model", 2),
        ("TESTERROR-stub_create_asset", 3),
("TESTERROR-stub_list_asset_model_properties", 4),
("TESTERROR-stub_put_asset_property_value", 4),
    ],
)
@pytest.mark.integ
def test_iot_sitewise_getting_started_error(mock_mgr, caplog, error, stop_on_index, monkeypatch):
    mock_mgr.setup_stubs(error, stop_on_index, mock_mgr.scenario_data.iot_sitewise_stubber, mock_mgr.scenario_data.cloud_formation_stubber, monkeypatch)

    with pytest.raises(ClientError) as exc_info:
        mock_mgr.scenario_data.scenario.run()
