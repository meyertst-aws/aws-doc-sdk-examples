// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/*
   A class containing functions that interact with AWS services.
*/

// snippet-start:[s3.swift.basics.handler]
// snippet-start:[s3.swift.basics.handler.imports]
import Foundation
import AWSS3
import ClientRuntime
import AWSClientRuntime
import Smithy
// snippet-end:[s3.swift.basics.handler.imports]

/// A class containing all the code that interacts with the AWS SDK for Swift.
public class ServiceHandler {
    let client: S3Client
    
    enum HandlerError: Error {
        case getObjectBody
        case readGetObjectBody
    }

    
    /// Initialize and return a new ``ServiceHandler`` object, which is used to drive the AWS calls
    /// used for the example.
    ///
    /// - Returns: A new ``ServiceHandler`` object, ready to be called to
    ///            execute AWS operations.
    // snippet-start:[s3.swift.basics.handler.init]
    public init() async throws {
        do {
            client = try await S3Client()
        } catch {
            print("ERROR: ", dump(error, name: "Initializing S3 client"))
            throw error
        }
    }
    // snippet-end:[s3.swift.basics.handler.init]

    /// Create a new user given the specified name.
    ///
    /// - Parameters:
    ///   - name: Name of the bucket to create.
    /// Throws an exception if an error occurs.
    // snippet-start:[s3.swift.basics.handler.createbucket]
    public func createBucket(name: String) async throws {
        let config = S3ClientTypes.CreateBucketConfiguration(
            locationConstraint: .usEast2
        )
        let input = CreateBucketInput(
            bucket: name,
            createBucketConfiguration: config
        )
        
        do {
            _ = try await client.createBucket(input: input)
        }
        catch (let error as BucketAlreadyOwnedByYou) {
            print("The bucket '\(name)' already exists and is owned by you. You may wish to ignore this exception.")
            throw error
        }
        catch {
            print("ERROR: ", dump(error, name: "Creating a bucket"))
            throw error

        }
    }
    // snippet-end:[s3.swift.basics.handler.createbucket]

    /// Delete a bucket.
    /// - Parameter name: Name of the bucket to delete.
    // snippet-start:[s3.swift.basics.handler.deletebucket]
    public func deleteBucket(name: String) async throws {
        let input = DeleteBucketInput(
            bucket: name
        )
        do {
            _ = try await client.deleteBucket(input: input)
        }
        catch {
            print("ERROR: ", dump(error, name: "Deleting a bucket"))
            throw error

        }
   }
    // snippet-end:[s3.swift.basics.handler.deletebucket]

    /// Upload a file from local storage to the bucket.
    /// - Parameters:
    ///   - bucket: Name of the bucket to upload the file to.
    ///   - key: Name of the file to create.
    ///   - file: Path name of the file to upload.
    // snippet-start:[s3.swift.basics.handler.uploadfile]
    public func uploadFile(bucket: String, key: String, file: String) async throws{
        let fileUrl = URL(fileURLWithPath: file)
        do {
        let fileData = try Data(contentsOf: fileUrl)
        let dataStream = ByteStream.data(fileData)
        
        let input = PutObjectInput(
            body: dataStream,
            bucket: bucket,
            key: key
        )
        

            _ = try await client.putObject(input: input)
        }
        catch {
            print("ERROR: ", dump(error, name: "Putting an object."))
            throw error
        }
    }
    // snippet-end:[s3.swift.basics.handler.uploadfile]

    /// Create a file in the specified bucket with the given name. The new
    /// file's contents are uploaded from a `Data` object.
    ///
    /// - Parameters:
    ///   - bucket: Name of the bucket to create a file in.
    ///   - key: Name of the file to create.
    ///   - data: A `Data` object to write into the new file.
    // snippet-start:[s3.swift.basics.handler.createfile]
    public func createFile(bucket: String, key: String, withData data: Data) async throws {
        let dataStream = ByteStream.data(data)
        
        let input = PutObjectInput(
            body: dataStream,
            bucket: bucket,
            key: key
        )
        
        do {
            _ = try await client.putObject(input: input)
        }
        catch {
            print("ERROR: ", dump(error, name: "Putting an object."))
            throw error
        }
}
    // snippet-end:[s3.swift.basics.handler.createfile]

    /// Download the named file to the given directory on the local device.
    ///
    /// - Parameters:
    ///   - bucket: Name of the bucket that contains the file to be copied.
    ///   - key: The name of the file to copy from the bucket.
    ///   - to: The path of the directory on the local device where you want to
    ///     download the file.
    // snippet-start:[s3.swift.basics.handler.downloadfile]
    public func downloadFile(bucket: String, key: String, to: String) async throws {
        let fileUrl = URL(fileURLWithPath: to).appendingPathComponent(key)

        let input = GetObjectInput(
            bucket: bucket,
            key: key
        )
        do {
            let output = try await client.getObject(input: input)


        guard let body = output.body else {
            throw HandlerError.getObjectBody
        }
        
        guard let data = try await body.readData() else {
            throw HandlerError.readGetObjectBody
        }
    
        
        try data.write(to: fileUrl)
        }
        catch {
            print("ERROR: ", dump(error, name: "Downloading a file."))
            throw error

        }
    }
    // snippet-end:[s3.swift.basics.handler.downloadfile]

    /// Read the specified file from the given S3 bucket into a Swift
    /// `Data` object.
    ///
    /// - Parameters:
    ///   - bucket: Name of the bucket containing the file to read.
    ///   - key: Name of the file within the bucket to read.
    ///
    /// - Returns: A `Data` object containing the complete file data.
    // snippet-start:[s3.swift.basics.handler.readfile]
    public func readFile(bucket: String, key: String) async throws -> Data {
        let input = GetObjectInput(
            bucket: bucket,
            key: key
        )
        let output = try await client.getObject(input: input)

        // Get the stream and return its contents in a `Data` object. If
        // there is no stream, return an empty `Data` object instead.
        guard let body = output.body,
              let data = try await body.readData() else {
            return "".data(using: .utf8)!
        }
        
        return data
    }
    // snippet-end:[s3.swift.basics.handler.readfile]

    /// Copy a file from one bucket to another.
    ///
    /// - Parameters:
    ///   - sourceBucket: Name of the bucket containing the source file.
    ///   - name: Name of the source file.
    ///   - destBucket: Name of the bucket to copy the file into.
    // snippet-start:[s3.swift.basics.handler.copyfile]
    public func copyFile(from sourceBucket: String, name: String, to destBucket: String) async throws {
        let srcUrl = ("\(sourceBucket)/\(name)").addingPercentEncoding(withAllowedCharacters: .urlPathAllowed)

        let input = CopyObjectInput(
            bucket: destBucket,
            copySource: srcUrl,
            key: name
        )
        _ = try await client.copyObject(input: input)
    }
    // snippet-end:[s3.swift.basics.handler.copyfile]

    /// Deletes the specified file from Amazon S3.
    ///
    /// - Parameters:
    ///   - bucket: Name of the bucket containing the file to delete.
    ///   - key: Name of the file to delete.
    ///
    // snippet-start:[s3.swift.basics.handler.deletefile]
    public func deleteFile(bucket: String, key: String) async throws {
        let input = DeleteObjectInput(
            bucket: bucket,
            key: key
        )

        do {
            _ = try await client.deleteObject(input: input)
        } catch {
            throw error
        }
    }
    // snippet-end:[s3.swift.basics.handler.deletefile]

    /// Returns an array of strings, each naming one file in the
    /// specified bucket.
    ///
    /// - Parameter bucket: Name of the bucket to get a file listing for.
    /// - Returns: An array of `String` objects, each giving the name of
    ///            one file contained in the bucket.
    // snippet-start:[s3.swift.basics.handler.listbucketfiles]
    public func listBucketFiles(bucket: String) async throws -> [String] {
        let input = ListObjectsV2Input(
            bucket: bucket
        )
        let output = try await client.listObjectsV2(input: input)
        var names: [String] = []

        guard let objList = output.contents else {
            return []
        }

        for obj in objList {
            if let objName = obj.key {
                names.append(objName)
            }
        }

        return names
    }
    // snippet-end:[s3.swift.basics.handler.listbucketfiles]
}
// snippet-end:[s3.swift.basics.handler]
