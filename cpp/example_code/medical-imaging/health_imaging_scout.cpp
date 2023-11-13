/*
   Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
   SPDX-License-Identifier: Apache-2.0
*/
/**
 * Before running this C++ code example, set up your development environment, including your credentials.
 *
 * For more information, see the following documentation topic:
 *
 * https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/getting-started.html
 *
 * For information on the structure of the code examples and how to build and run the examples, see
 * https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/getting-started-code-examples.html.
 *
 **/

#include <aws/core/Aws.h>
#include <aws/cloudformation/CloudFormationClient.h>
#include <aws/cloudformation/model/CreateStackRequest.h>
#include <aws/cloudformation/model/DeleteStackRequest.h>
#include <aws/cloudformation/model/DescribeStacksRequest.h>
#include <aws/medical-imaging/MedicalImagingClient.h>
#include <aws/medical-imaging/model/StartDICOMImportJobRequest.h>
#include <aws/medical-imaging/model/GetDICOMImportJobRequest.h>
#include <aws/core/utils/UUID.h>
#include <fstream>

const char STACK_NAME[] = "health-imaging-scout";

bool waitStackCreated(Aws::CloudFormation::CloudFormationClient &cloudFormationClient, const std::string &stackName,
                      Aws::Vector<Aws::CloudFormation::Model::Output>& outputs)
{
    Aws::CloudFormation::Model::DescribeStacksRequest describeStacksRequest;
    describeStacksRequest.SetStackName(stackName);
    Aws::CloudFormation::Model::StackStatus stackStatus = Aws::CloudFormation::Model::StackStatus::CREATE_IN_PROGRESS;

    while (stackStatus == Aws::CloudFormation::Model::StackStatus::CREATE_IN_PROGRESS)
    {
        stackStatus = Aws::CloudFormation::Model::StackStatus::NOT_SET;
        auto outcome = cloudFormationClient.DescribeStacks(describeStacksRequest);
        if (outcome.IsSuccess())
        {
            const auto& stacks = outcome.GetResult().GetStacks();
            if (!stacks.empty()) {
                const auto &stack = stacks[0];
                stackStatus = stack.GetStackStatus();
                if (stackStatus ==
                    Aws::CloudFormation::Model::StackStatus::CREATE_COMPLETE) {
                    outputs = stack.GetOutputs();
                }
                else if (stackStatus != Aws::CloudFormation::Model::StackStatus::CREATE_IN_PROGRESS) {
                    std::cerr << "Failed to create stack because "
                              << stack.GetStackStatusReason() << std::endl;
                }
            }
        }
    }

    if (stackStatus == Aws::CloudFormation::Model::StackStatus::CREATE_COMPLETE)
    {
        std::cout << "Stack creation completed." << std::endl;
    }

    return stackStatus == Aws::CloudFormation::Model::StackStatus::CREATE_COMPLETE;
}

bool waitStackDeleted(Aws::CloudFormation::CloudFormationClient &cloudFormationClient, const std::string &stackName)
{
    Aws::CloudFormation::Model::DescribeStacksRequest describeStacksRequest;
    describeStacksRequest.SetStackName(stackName);
    Aws::CloudFormation::Model::StackStatus stackStatus = Aws::CloudFormation::Model::StackStatus::DELETE_IN_PROGRESS;

    while (stackStatus == Aws::CloudFormation::Model::StackStatus::DELETE_IN_PROGRESS)
    {
        stackStatus = Aws::CloudFormation::Model::StackStatus::NOT_SET;
        auto outcome = cloudFormationClient.DescribeStacks(describeStacksRequest);
        if (outcome.IsSuccess())
        {
            const auto& stacks = outcome.GetResult().GetStacks();
            if (!stacks.empty())
            {
                const auto &stack = stacks[0];
                stackStatus = stack.GetStackStatus();
                if (stackStatus != Aws::CloudFormation::Model::StackStatus::DELETE_IN_PROGRESS  &&
                    stackStatus != Aws::CloudFormation::Model::StackStatus::DELETE_COMPLETE)
                {
                    std::cerr << "Failed to delete stack because " << stack.GetStackStatusReason() << std::endl;
                }

            }
            else {
                stackStatus = Aws::CloudFormation::Model::StackStatus::DELETE_COMPLETE;
            }

        }
        else{
            auto&  error = outcome.GetError();
            if (error.GetResponseCode() == Aws::Http::HttpResponseCode::BAD_REQUEST &&
            (outcome.GetError().GetMessage().find("does not exist") != std::string::npos))
            {
                stackStatus = Aws::CloudFormation::Model::StackStatus::DELETE_COMPLETE;
            }
            else {
                std::cerr << "Failed to describe stack. "
                          << outcome.GetError().GetMessage() << std::endl;
            }
        }
    }

    if (stackStatus == Aws::CloudFormation::Model::StackStatus::DELETE_COMPLETE)
    {
        std::cout << "Stack deletion completed." << std::endl;
    }

    return stackStatus == Aws::CloudFormation::Model::StackStatus::DELETE_COMPLETE;
}

bool startDicomImport(const Aws::MedicalImaging::MedicalImagingClient &medicalImagingClient,
                          const Aws::String &dataStoreArn, const Aws::String &bucketName,

bool CreateImportStack(Aws::CloudFormation::CloudFormationClient &cloudFormationClient)
{
     Aws::CloudFormation::Model::CreateStackRequest createStackRequest;
    createStackRequest.SetStackName(STACK_NAME);
    createStackRequest.SetOnFailure(Aws::CloudFormation::Model::OnFailure::DELETE_);
    std::ifstream ifstream("/Users/meyertst/Development/aws-doc-sdk-examples/cpp/example_code/medical-imaging/CfnImportBucketTemplate.json");

    if (!ifstream)
    {
        std::cerr << "Failed to open file" << std::endl;
        return "";
    }

    std::stringstream stringstream;
    stringstream << ifstream.rdbuf();
    createStackRequest.SetTemplateBody(stringstream.str());

    std::string bucketName = "health-imaging-import-";
    bucketName += (Aws::Utils::UUID::RandomUUID());
    if (bucketName.length() > 63)
    {
        bucketName = bucketName.substr(0, 63);
    }

    std::transform(bucketName.begin(), bucketName.end(), bucketName.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    std::string bucketTag("BucketName");
    std::string policyName = "health-imaging-import-bucket-";
    policyName += (Aws::Utils::UUID::RandomUUID());
    createStackRequest.SetParameters(
            { Aws::CloudFormation::Model::Parameter().WithParameterKey("BucketName").WithParameterValue(bucketName),
              Aws::CloudFormation::Model::Parameter().WithParameterKey("PolicyName").WithParameterValue(policyName),
              Aws::CloudFormation::Model::Parameter().WithParameterKey("DatastoreArn").WithParameterValue("arn:aws:medical-imaging:us-east-1:123502194722:datastore/b62634992c0e44079cd6bcab0210d464"),
              Aws::CloudFormation::Model::Parameter().WithParameterKey("UserID").WithParameterValue("123502194722")});
    createStackRequest.SetCapabilities({Aws::CloudFormation::Model::Capability::CAPABILITY_IAM});

    auto outcome = cloudFormationClient.CreateStack(createStackRequest);
    bool result = false;
    Aws::Vector<Aws::CloudFormation::Model::Output> outputs;
    if (outcome.IsSuccess())
    {
        std::cout << "Stack creation initiated." << std::endl;
        result = waitStackCreated(cloudFormationClient, STACK_NAME, outputs);
    }
    else {
        std::cerr << "Failed to create stack" << outcome.GetError().GetMessage()
                  << std::endl;
    }

    Aws::String roleArn;
    Aws::String bucketArn;
    if (!outputs.empty()) {
        for (auto &output : outputs) {
            if (output.GetOutputKey() == "BucketArn") {
                bucketArn = output.GetOutputValue();
            }
            else if (output.GetOutputKey() == "RoleArn") {
                roleArn = output.GetOutputValue();
            }
        }
    }
    else {
        std::cerr <<  "Failed to get stack outputs" << std::endl;
    }

    if (!roleArn.empty() && !bucketArn.empty()) {

    }

    return result;
}

bool deleteStack(Aws::CloudFormation::CloudFormationClient &cloudFormationClient, const std::string &stackName)
{
    Aws::CloudFormation::Model::DeleteStackRequest deleteStackRequest;
    deleteStackRequest.SetStackName(stackName);
    auto outcome = cloudFormationClient.DeleteStack(deleteStackRequest);
    bool result = false;
    if (outcome.IsSuccess())
    {
        std::cout << "Stack deletion initiated." << std::endl;
        result = waitStackDeleted(cloudFormationClient, stackName);
    }
    else {
        std::cerr << "Failed to delete stack" << outcome.GetError().GetMessage()
                << std::endl;
    }

    return result;
}


int main(int argc, char **argv) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::CloudFormation::CloudFormationClient cloudFormationClient;
        CreateImportStack(cloudFormationClient);
        deleteStack(cloudFormationClient, STACK_NAME);
    }
    Aws::ShutdownAPI(options);

    return 0;
}
