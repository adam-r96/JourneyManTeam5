/*
Copyright 2024 Adobe. All rights reserved.
This file is licensed to you under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License. You may obtain a copy
of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
OF ANY KIND, either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

//! @file sendusd.h
//! @brief Base application type for usd update and import
//! @author Daniel Stover - Adobe
//! @date 20240828
//! @copyright Adobe. All rights reserved.

#pragma once

#include <substance/connector/framework/application.h>
#include <substance/connector/framework/details/sendtoschema.h>
#include <vector>

namespace Substance
{
namespace Connector
{
namespace Framework
{
class UsdApplication : public Application
{
public:
	//! @brief Default constructor
	UsdApplication();

	//! @brief Destructor
	virtual ~UsdApplication();

	//! @brief Run live UsdApplication preinitialization steps
	void preInit() override;

	//! @brief Run live UsdApplication postinitialization
	void postInit() override;

	//! @brief Run live UsdApplication postshutdown
	void postShutdown() override;

	//! @brief Returns the list of feature ids for this application
	const std::vector<substance_connector_uuid_t> getFeatureIds() override;

	//! @brief Write an send message to a given connection
	void sendLoadUsd(unsigned int context, const char* message);

	//! @brief Write an export message to a given connection
	void sendLoadUsd(unsigned int context, Details::send_to_schema& schema);

public:
	static const substance_connector_uuid_t sLoadUsdId;
	static const substance_connector_uuid_t sUpdateUsdId;

	MessageCallback mRecvLoadUsd;
	MessageCallback mRecvUpdateUsd;

	size_t mUserData;
};
} // namespace Framework
} // namespace Connector
} // namespace Substance
