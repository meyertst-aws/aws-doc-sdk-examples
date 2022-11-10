/*
   Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
   SPDX-License-Identifier: Apache-2.0
*/

/**
 *  SESV2EmailHandler.h/.cpp
 *
 *  The code in these 2 file implements the sending of a multipart email message containing
 *  plain text, HTML text, and an attachment file using Amazon Simple Email Service
 *  (Amazon SES).
 *
 * To run the example, refer to the instructions in the ReadMe.
 */

#pragma once
#ifndef SERVERLESSAURORA_SESV2EMAILHANDLER_H
#define SERVERLESSAURORA_SESV2EMAILHANDLER_H

#include "ItemTrackerHTTPHandler.h"
#include <aws/core/client/ClientConfiguration.h>

namespace AwsDoc {
    namespace CrossService {
        /**
         *  SESV2EmailHandler
         *
         *  Implementation of SESEmailReceiver which sends emails using Amazon SES.
         *
         */
        class SESV2EmailHandler : public SESEmailReceiver {
        public :

            //! SESV2EmailHandler constructor.
            /*!
             \sa SESV2EmailHandler::SESV2EmailHandler()
             \param fromEmailAddress: Verified mail address enabled in Amazon SES.
             \param clientConfiguration: Aws client configuration.
             */
            explicit SESV2EmailHandler(const Aws::String &fromEmailAddress,
                                       const Aws::Client::ClientConfiguration &clientConfiguration);

            //! Routine which sends an email.
            /*!
             \sa SESV2EmailHandler::sendEmail()
             \param emailAddress: The destination email address.
             \param workItems: List of work items for the email content.
             \return bool: Successful completion.
             */
            virtual bool sendEmail(const Aws::String emailAddress,
                                   const std::vector<WorkItem> &workItems) override;

        private:

            //! Routine which writes the header of a multipart raw email message.
            /*!
             \sa SESV2EmailHandler::writeMultipartHeader()
             \param toEmail: The destination email address.
             \param subject: The email subject.
             \param returnPath: Optional return email address.
             \param ostream: An output stream.
             \return void:
             */
            void writeMultipartHeader(const Aws::String &toEmail,
                                      const Aws::String &subject,
                                      const Aws::String &returnPath,
                                      std::ostream &ostream);

            //! Routine which writes the plain text part of a multipart raw email message.
            /*!
             \sa SESV2EmailHandler::writePlainTextPart()
             \param plainText: Plain text content.
             \param ostream: An output stream.
             \return void:
             */
            static void
            writePlainTextPart(const Aws::String &plainText, std::ostream &ostream);

            //! Routine which writes the HTML text part of a multipart raw email message.
            /*!
             \sa SESV2EmailHandler::writeHtmlTextPart()
             \param htmlText: Content in HTML text format.
             \param ostream: An output stream.
             \return void:
             */
            static void writeHtmlTextPart(const Aws::String &htmlText, std::ostream &ostream);

            //! Routine which writes the file attachment part of a multipart raw email message.
            /*!
             \sa SESV2EmailHandler::writeAttachmentPart()
             \param contentType: The MIME content type.
             \param name: The file name.
             \param attachmentBuffer: Buffer containing the file contents                                             cv f.
             \param ostream: An output stream.
             \return void:
             */
            static void
            writeAttachmentPart(const Aws::String &contentType, const Aws::String &name,
                                const std::vector<unsigned char> &attachmentBuffer,
                                std::ostream &ostream);

            Aws::Client::ClientConfiguration mClientConfiguration;
            Aws::String mFromEmailAddress;
        };
    }  // namespace CrossService
} // namespace AwsDoc

#endif //SERVERLESSAURORA_SESV2EMAILHANDLER_H
