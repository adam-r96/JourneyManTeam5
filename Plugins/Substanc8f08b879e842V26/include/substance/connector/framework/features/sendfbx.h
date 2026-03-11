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

//! @file sendfbx.h
//! @brief Base application type for fbx update and support
//! @author Josh Green - Adobe
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
class FbxApplication : public Application
{
public:
	//! @brief Default constructor
	FbxApplication();

	//! @brief Destructor
	virtual ~FbxApplication();

	//! @brief Run live FbxApplication preinitialization steps
	void preInit() override;

	//! @brief Run live FbxApplication postinitialization
	void postInit() override;

	//! @brief Run live FbxApplication postshutdown
	void postShutdown() override;

	//! @brief Returns the list of feature ids for this application
	const std::vector<substance_connector_uuid_t> getFeatureIds() override;

	//! @brief Send to another connection a message to load an fbx
	//! @param context Integer representing the underlying connection
	//! @param message File path of the fbx for the other application to load
	void sendLoadFbx(unsigned int context, const char* message);

	//! @brief Send to another application a message to load the fbx
	//! @param context Integer representing the underlying connection
	//! @param message File path of the fbx for the other application to load
	void sendLoadFbx(unsigned int context, Details::send_to_schema& message);


public:
	static const substance_connector_uuid_t sLoadFbxId;
	static const substance_connector_uuid_t sUpdateFbxId;

	MessageCallback mRecvLoadFbx;
	MessageCallback mRecvUpdateFbx;

	size_t mUserData;
};
} // namespace Framework
} // namespace Connector
} // namespace Substance
