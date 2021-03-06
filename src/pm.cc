/*******************************************************************************
 *  Code contributed to the webinos project
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  
 *     http://www.apache.org/licenses/LICENSE-2.0
 *  
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Copyright 2011 Telecom Italia SpA
 * 
 ******************************************************************************/

#include <v8.h>
#include <node.h>

#include "core/policymanager/PolicyManager.h"
#include "debug.h"

using namespace node;
using namespace v8;


class PolicyManagerInt: ObjectWrap{

private:  
	int m_count;
	
public:
	PolicyManager* pminst;
	string policyFileName;
	static Persistent<FunctionTemplate> s_ct;
  
	static void Init(Handle<Object> target)  {
		HandleScope scope;
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		s_ct = Persistent<FunctionTemplate>::New(t);
		s_ct->InstanceTemplate()->SetInternalFieldCount(1);
		s_ct->SetClassName(String::NewSymbol("PolicyManagerInt"));
		NODE_SET_PROTOTYPE_METHOD(s_ct, "enforceRequest", EnforceRequest);
		NODE_SET_PROTOTYPE_METHOD(s_ct, "reloadPolicy", ReloadPolicy);
		NODE_SET_PROTOTYPE_METHOD(s_ct, "getPolicyFilename", GetPolicyFilename);
		target->Set(String::NewSymbol("PolicyManagerInt"),
		s_ct->GetFunction());
	}

	PolicyManagerInt() :    m_count(0)  {
	}
	
	~PolicyManagerInt()  {  }

	static Handle<Value> New(const Arguments& args)  {
		HandleScope scope;
		PolicyManagerInt* pmtmp = new PolicyManagerInt();

		map<string, vector<string>*> *pip = new map<string, vector<string>*>();
		(*pip)["http://webinos.org/subject/id/PZ-Owner"] = new vector<string>();
		(*pip)["http://webinos.org/subject/id/known"] = new vector<string>();

		if (args.Length() > 1) {
			if (!args[0]->IsString()) {
				LOGD("Wrong parameter type");
				return ThrowException(Exception::TypeError(String::New("Bad type argument")));
			}
			if (!args[1]->IsObject()) {
				LOGD("Wrong parameter type");
				return ThrowException(Exception::TypeError(String::New("Bad type argument")));
			}
			v8::String::AsciiValue tmpFileName(args[0]->ToString());
			LOGD("Parameter file: %s", *tmpFileName);
			//if(pmtmp->policyFileName) {
			//	delete[] pmtmp->policyFileName;
			//}
			pmtmp->policyFileName = *tmpFileName;

			if (args[1]->ToObject()->Has(String::New("http://webinos.org/subject/id/PZ-Owner"))) {
				v8::String::AsciiValue ownerId(args[1]->ToObject()->Get(String::New("http://webinos.org/subject/id/PZ-Owner")));
				(*pip)["http://webinos.org/subject/id/PZ-Owner"]->push_back(*ownerId);
			}

			if (args[1]->ToObject()->Has(String::New("http://webinos.org/subject/id/known"))) {
				v8::Local<Array> knownList = v8::Local<Array>::Cast(args[1]->ToObject()->Get(String::New("http://webinos.org/subject/id/known")));
                LOGD("knownList->Length() = %d", knownList->Length());
				unsigned int i = 0;
				for (i=0; i < knownList->Length(); i++)
				{
			        LOGD("Known[%d]:",i);
					v8::String::AsciiValue knownId(knownList->Get(i));
					(*pip)["http://webinos.org/subject/id/known"]->push_back(*knownId);

			        LOGD("Known[%d]: %s",i, *knownId);
				}
			}
		}
		else {
			LOGD("Missing parameter");
			return ThrowException(Exception::TypeError(String::New("Missing argument")));
		}

		pmtmp->pminst = new PolicyManager(pmtmp->policyFileName, pip);
		pmtmp->Wrap(args.This());
		return args.This();
	}

	static Handle<Value> EnforceRequest(const Arguments& args)  {
		HandleScope scope;

		if (args.Length() < 1) {
			return ThrowException(Exception::TypeError(String::New("Argument missing")));
		}

		if (!args[0]->IsObject()) {
			return ThrowException(Exception::TypeError(String::New("Bad type argument")));
		}
		
		PolicyManagerInt* pmtmp = ObjectWrap::Unwrap<PolicyManagerInt>(args.This());
		pmtmp->m_count++;

		map<string, vector<string>*> * subject_attrs = new map<string, vector<string>*>();
		(*subject_attrs)["user-id"] = new vector<string>();
		(*subject_attrs)["user-key-cn"] = new vector<string>();
		(*subject_attrs)["user-key-fingerprint"] = new vector<string>();
		(*subject_attrs)["user-key-root-cn"] = new vector<string>();
		(*subject_attrs)["user-key-root-fingerprint"] = new vector<string>();
		
		(*subject_attrs)["id"] = new vector<string>();
		
		(*subject_attrs)["distributor-key-cn"] = new vector<string>();
		(*subject_attrs)["distributor-key-fingerprint"] = new vector<string>();
		(*subject_attrs)["distributor-key-root-cn"] = new vector<string>();
		(*subject_attrs)["distributor-key-root-fingerprint"] = new vector<string>();

		(*subject_attrs)["author-key-cn"] = new vector<string>();
		(*subject_attrs)["author-key-fingerprint"] = new vector<string>();
		(*subject_attrs)["author-key-root-cn"] = new vector<string>();
		(*subject_attrs)["author-key-root-fingerprint"] = new vector<string>();

		(*subject_attrs)["target-id"] = new vector<string>();
		(*subject_attrs)["target-domain"] = new vector<string>();
		(*subject_attrs)["requestor-id"] = new vector<string>();
		(*subject_attrs)["requestor-domain"] = new vector<string>();
		(*subject_attrs)["webinos-enabled"] = new vector<string>();

		map<string, vector<string>*> * resource_attrs = new map<string, vector<string>*>();
		(*resource_attrs)["api-feature"] = new vector<string>();
		(*resource_attrs)["service-id"] = new vector<string>();
		(*resource_attrs)["device-cap"] = new vector<string>();
		(*resource_attrs)["param:feature"] = new vector<string>();

		if (args[0]->ToObject()->Has(String::New("resourceInfo"))) {
			v8::Local<Value> riTmp = args[0]->ToObject()->Get(String::New("resourceInfo"));
			if (riTmp->ToObject()->Has(String::New("deviceCap"))) {
				v8::String::AsciiValue deviceCap(riTmp->ToObject()->Get(String::New("deviceCap")));
				(*resource_attrs)["device-cap"]->push_back(*deviceCap);
				LOGD("Parameter device-cap : %s", *deviceCap);
			}
			if (riTmp->ToObject()->Has(String::New("apiFeature"))) {
				v8::String::AsciiValue apiFeature(riTmp->ToObject()->Get(String::New("apiFeature")));
				(*resource_attrs)["api-feature"]->push_back(*apiFeature);
				LOGD("Parameter api-feature : %s", *apiFeature);
			}
			if (riTmp->ToObject()->Has(String::New("serviceId"))) {
				v8::String::AsciiValue serviceId(riTmp->ToObject()->Get(String::New("serviceId")));
				(*resource_attrs)["service-id"]->push_back(*serviceId);
				LOGD("Parameter service-id : %s", *serviceId);
			}
			if (riTmp->ToObject()->Has(String::New("paramFeature"))) {
				v8::String::AsciiValue paramFeature(riTmp->ToObject()->Get(String::New("paramFeature")));
				(*resource_attrs)["param:feature"]->push_back(*paramFeature);
				LOGD("Parameter param:feature : %s", *paramFeature);
			}
		}
		
		if (args[0]->ToObject()->Has(String::New("subjectInfo"))) {
			v8::Local<Value> siTmp = args[0]->ToObject()->Get(String::New("subjectInfo"));
			if (siTmp->ToObject()->Has(String::New("userId"))) {
				v8::String::AsciiValue userId(siTmp->ToObject()->Get(String::New("userId")));
				(*subject_attrs)["user-id"]->push_back(*userId);
				LOGD("Parameter user-id : %s", *userId);
			}
			if (siTmp->ToObject()->Has(String::New("userKeyCn"))) {
				v8::String::AsciiValue userKeyCn(siTmp->ToObject()->Get(String::New("userKeyCn")));
				(*subject_attrs)["user-key-cn"]->push_back(*userKeyCn);
				LOGD("Parameter user-key-cn : %s", *userKeyCn);
			}
			if (siTmp->ToObject()->Has(String::New("userKeyFingerprint"))) {
				v8::String::AsciiValue userKeyFingerprint(siTmp->ToObject()->Get(String::New("userKeyFingerprint")));
				(*subject_attrs)["user-key-fingerprint"]->push_back(*userKeyFingerprint);
				LOGD("Parameter user-key-fingerprint : %s", *userKeyFingerprint);
			}
			if (siTmp->ToObject()->Has(String::New("userKeyRootCn"))) {
				v8::String::AsciiValue userKeyRootCn(siTmp->ToObject()->Get(String::New("userKeyRootCn")));
				(*subject_attrs)["user-key-root-cn"]->push_back(*userKeyRootCn);
				LOGD("Parameter user-key-root-cn : %s", *userKeyRootCn);
			}
			if (siTmp->ToObject()->Has(String::New("userKeyRootFingerprint"))) {
				v8::String::AsciiValue userKeyRootFingerprint(siTmp->ToObject()->Get(String::New("userKeyRootFingerprint")));
				(*subject_attrs)["user-key-root-fingerprint"]->push_back(*userKeyRootFingerprint);
				LOGD("Parameter user-key-root-fingerprint : %s", *userKeyRootFingerprint);
			}
		}

		if (args[0]->ToObject()->Has(String::New("widgetInfo"))) {
			v8::Local<Value> wiTmp = args[0]->ToObject()->Get(String::New("widgetInfo"));
			if (wiTmp->ToObject()->Has(String::New("id"))) {
				v8::String::AsciiValue id(wiTmp->ToObject()->Get(String::New("id")));
				(*subject_attrs)["id"]->push_back(*id);
				LOGD("Parameter id : %s", *id);
			}
			if (wiTmp->ToObject()->Has(String::New("distributorKeyCn"))) {
				v8::String::AsciiValue distributorKeyCn(wiTmp->ToObject()->Get(String::New("distributorKeyCn")));
				(*subject_attrs)["distributor-key-cn"]->push_back(*distributorKeyCn);
				LOGD("Parameter distributor-key-cn : %s", *distributorKeyCn);
			}
			if (wiTmp->ToObject()->Has(String::New("distributorKeyFingerprint"))) {
				v8::String::AsciiValue distributorKeyFingerprint(wiTmp->ToObject()->Get(String::New("distributorKeyFingerprint")));
				(*subject_attrs)["distributor-key-fingerprint"]->push_back(*distributorKeyFingerprint);
				LOGD("Parameter distributor-key-fingerprint : %s", *distributorKeyFingerprint);
			}
			if (wiTmp->ToObject()->Has(String::New("distributorKeyRootCn"))) {
				v8::String::AsciiValue distributorKeyRootCn(wiTmp->ToObject()->Get(String::New("distributorKeyRootCn")));
				(*subject_attrs)["distributor-key-root-cn"]->push_back(*distributorKeyRootCn);
				LOGD("Parameter distributor-key-root-cn : %s", *distributorKeyRootCn);
			}
			if (wiTmp->ToObject()->Has(String::New("distributorKeyRootFingerprint"))) {
				v8::String::AsciiValue distributorKeyRootFingerprint(wiTmp->ToObject()->Get(String::New("distributorKeyRootFingerprint")));
				(*subject_attrs)["distributor-key-root-fingerprint"]->push_back(*distributorKeyRootFingerprint);
				LOGD("Parameter distributor-key-root-fingerprint : %s", *distributorKeyRootFingerprint);
			}
			if (wiTmp->ToObject()->Has(String::New("authorKeyCn"))) {
				v8::String::AsciiValue authorKeyCn(wiTmp->ToObject()->Get(String::New("authorKeyCn")));
				(*subject_attrs)["author-key-cn"]->push_back(*authorKeyCn);
				LOGD("Parameter author-key-cn : %s", *authorKeyCn);
			}
			if (wiTmp->ToObject()->Has(String::New("authorKeyFingerprint"))) {
				v8::String::AsciiValue authorKeyFingerprint(wiTmp->ToObject()->Get(String::New("authorKeyFingerprint")));
				(*subject_attrs)["author-key-fingerprint"]->push_back(*authorKeyFingerprint);
				LOGD("Parameter author-key-fingerprint : %s", *authorKeyFingerprint);
			}
			if (wiTmp->ToObject()->Has(String::New("authorKeyRootCn"))) {
				v8::String::AsciiValue authorKeyRootCn(wiTmp->ToObject()->Get(String::New("authorKeyRootCn")));
				(*subject_attrs)["author-key-root-cn"]->push_back(*authorKeyRootCn);
				LOGD("Parameter author-key-root-cn : %s", *authorKeyRootCn);
			}
			if (wiTmp->ToObject()->Has(String::New("authorKeyRootFingerprint"))) {
				v8::String::AsciiValue authorKeyRootFingerprint(wiTmp->ToObject()->Get(String::New("authorKeyRootFingerprint")));
				(*subject_attrs)["author-key-root-fingerprint"]->push_back(*authorKeyRootFingerprint);
				LOGD("Parameter author-key-root-fingerprint : %s", *authorKeyRootFingerprint);
			}
		}

		if (args[0]->ToObject()->Has(String::New("deviceInfo"))) {
			v8::Local<Value> diTmp = args[0]->ToObject()->Get(String::New("deviceInfo"));
			if (diTmp->ToObject()->Has(String::New("targetId"))) {
				v8::String::AsciiValue targetId(diTmp->ToObject()->Get(String::New("targetId")));
				(*subject_attrs)["target-id"]->push_back(*targetId);
				LOGD("Parameter target-id : %s", *targetId);
			}
			if (diTmp->ToObject()->Has(String::New("targetDomain"))) {
				v8::String::AsciiValue targetDomain(diTmp->ToObject()->Get(String::New("targetDomain")));
				(*subject_attrs)["target-domain"]->push_back(*targetDomain);
				LOGD("Parameter target-domain : %s", *targetDomain);
			}
			if (diTmp->ToObject()->Has(String::New("requestorId"))) {
				v8::String::AsciiValue requestorId(diTmp->ToObject()->Get(String::New("requestorId")));
				(*subject_attrs)["requestor-id"]->push_back(*requestorId);
				LOGD("Parameter requestor-id : %s", *requestorId);
			}
			if (diTmp->ToObject()->Has(String::New("requestorDomain"))) {
				v8::String::AsciiValue requestorDomain(diTmp->ToObject()->Get(String::New("requestorDomain")));
				(*subject_attrs)["requestor-domain"]->push_back(*requestorDomain);
				LOGD("Parameter requestor-domain : %s", *requestorDomain);
			}
			if (diTmp->ToObject()->Has(String::New("webinosEnabled"))) {
				v8::String::AsciiValue webinosEnabled(diTmp->ToObject()->Get(String::New("webinosEnabled")));
				(*subject_attrs)["webinos-enabled"]->push_back(*webinosEnabled);
				LOGD("Parameter webinos-enabled : %s", *webinosEnabled);
			}
		}

		vector<bool> purpose;
		if (args[0]->ToObject()->Has(String::New("purpose"))) {
			if (args[0]->ToObject()->Get(String::New("purpose"))->IsArray()) {
				v8::Local<Array> pTmp = v8::Local<Array>::Cast(args[0]->ToObject()->Get(String::New("purpose")));
				LOGD("DHPref: read %d purposes", pTmp->Length());
				if (pTmp->Length() == arraysize(ontology_vector)) {
					for(unsigned int i = 0; i < arraysize(ontology_vector); i++) {
						if (pTmp->Get(i)->BooleanValue() == true) {
							LOGD("DHPref: purpose number %d is true", i);
							purpose.push_back(pTmp->Get(i)->BooleanValue());
						}
						else if (pTmp->Get(i)->BooleanValue() == false) {
							LOGD("DHPref: purpose number %d is false", i);
							purpose.push_back(pTmp->Get(i)->BooleanValue());
						}
						else {
							// invalid purpose vector
							LOGD("DHPref: purpose number %d is undefined", i);
							purpose.clear();
							break;
						}
					}
				}
				else {
					LOGD("DHPref: invalid purpose parameter, wrong vector length");
				}
			}
			else {
				LOGD("DHPref: invalid purpose parameter, it is not an array");
			}
		}
		else {
			LOGD("DHPref: purpose parameter not found");
			purpose = vector<bool>(arraysize(ontology_vector), true);
			LOGD("DHPref: default purpose parameter generation (all purposes required)");
		}

		obligations obs;
		obligation ob;
		map<string, string> action;
		map<string, string> trigger;
		vector< map<string, string> > triggers;
		v8::Local<Value> actTmp, triggerTmp;
		v8::Local<Array> triggersTmp;

		if (args[0]->ToObject()->Has(String::New("obligations"))) {
			if (args[0]->ToObject()->Get(String::New("obligations"))->IsArray()) {
				v8::Local<Array> obTmp = v8::Local<Array>::Cast(args[0]->ToObject()->Get(String::New("obligations")));
				LOGD("DHPref: read %d obligations", obTmp->Length());
				for (unsigned int i = 0; i < obTmp->Length(); i++) {
					if (obTmp->Get(i)->ToObject()->Has(String::New("action"))) {
						LOGD("Obligation %d: action found", i);
						actTmp = obTmp->Get(i)->ToObject()->Get(String::New("action"));
						if (actTmp->ToObject()->Has(String::New(actionIdTag.c_str()))) {
							v8::String::AsciiValue actionID(actTmp->ToObject()->Get(String::New(actionIdTag.c_str())));
							action[actionIdTag]=*actionID;
							LOGD("Obligation %d: actionID %s", i, *actionID);

							// ActionNotifyDataSubject
							if (strcmp(*actionID, actionNotifyTag.c_str()) == 0) {
								// Media paramter
								if (actTmp->ToObject()->Has(String::New(mediaTag.c_str()))) {
									v8::String::AsciiValue media(actTmp->ToObject()->Get(String::New(mediaTag.c_str())));
									action[mediaTag]=*media;
									LOGD("Obligation %d: Media %s", i, *media);
								}
								else {
									// invalid action: Media required
									LOGD("Obligation %d: Media is missing", i);
									action.clear();
									continue;
								}
								// Address paramter
								if (actTmp->ToObject()->Has(String::New(addressTag.c_str()))) {
									v8::String::AsciiValue address(actTmp->ToObject()->Get(String::New(addressTag.c_str())));
									action[addressTag]=*address;
									LOGD("Obligation %d: Address %s", i, *address);
								}
								else {
									// invalid action: Address required
									LOGD("Obligation %d: Address is missing", i);
									action.clear();
									continue;
								}
							}
							// other actions
							else if (strcmp(*actionID, actionDeleteTag.c_str()) != 0 &&
								strcmp(*actionID, actionAnonymizeTag.c_str()) != 0 &&
								strcmp(*actionID, actionLogTag.c_str()) != 0 && 
								strcmp(*actionID, actionSecureLogTag.c_str()) != 0) {

								// invalid action: unrecognized actionID
								LOGD("Obligation %d: unrecognized actionID %s", i, *actionID);
								action.clear();
								continue;
							}
						}
						else {
							// invalid action: actionID field required
							LOGD("Obligation %d: actionID is missing", i);
							continue;
						}
					}
					else {
						// invalid obligation: action required
						LOGD("Obligation %d: action is missing", i);
						continue;
					}
					if (obTmp->Get(i)->ToObject()->Has(String::New("triggers"))) {
						if (obTmp->Get(i)->ToObject()->Get(String::New("triggers"))->IsArray()) {
							triggersTmp = v8::Local<Array>::Cast(obTmp->Get(i)->ToObject()->Get(String::New("triggers")));
							LOGD("Obligation %d: %d triggers found", i, triggersTmp->Length());
							for (unsigned int j = 0; j < triggersTmp->Length(); j++) {
								if (triggersTmp->Get(j)->ToObject()->Has(String::New(triggerIdTag.c_str()))) {
									v8::String::AsciiValue triggerID(triggersTmp->Get(j)->ToObject()->Get(String::New(triggerIdTag.c_str())));
									trigger[triggerIdTag]=*triggerID;
									LOGD("Obligation %d, trigger %d: actionID %s", i, j, *triggerID);
									triggerTmp = triggersTmp->Get(j);
									// TriggerAtTime
									if (strcmp(*triggerID, triggerAtTimeTag.c_str()) == 0) {
										// Start
										if (triggerTmp->ToObject()->Has(String::New(startTag.c_str()))){
											v8::String::AsciiValue start(triggerTmp->ToObject()->Get(String::New(startTag.c_str())));
											trigger[startTag]=*start;
											LOGD("Obligation %d, trigger %d: Start %s", i, j, *start);
										}
										else {
											// invalid trigger: Start required
											LOGD("Obligation %d, trigger %d: Start is missing", i, j);
											trigger.clear();
											continue;
										}
										// MaxDelay
										if (triggerTmp->ToObject()->Has(String::New(maxDelayTag.c_str()))){
											v8::String::AsciiValue maxdelay(triggerTmp->ToObject()->Get(String::New(maxDelayTag.c_str())));
											trigger[maxDelayTag]=*maxdelay;
											LOGD("Obligation %d, trigger %d: MaxDelay %s", i, j, *maxdelay);
										}
										else {
											// invalid trigger: MaxDelay required
											LOGD("Obligation %d, trigger %d: MaxDelay is missing", i, j);
											trigger.clear();
											continue;
										}
									}
									// TriggerPersonalDataAccessedForPurpose
									else if (strcmp(*triggerID, triggerPersonalDataAccessedTag.c_str()) == 0) {

										string purposes;
										while (purposes.size() < arraysize(ontology_vector))
											purposes.append("0");

										// Purpose
										if (triggerTmp->ToObject()->Has(String::New(purposeTag.c_str()))){
											v8::Local<Array> pTmp = v8::Local<Array>::Cast(triggerTmp->ToObject()->Get(String::New(purposeTag.c_str())));
											LOGD("Obligation %d, trigger %d: read %d puroses", i, j, pTmp->Length());
											if (pTmp->Length() == arraysize(ontology_vector)) {
												for(unsigned int k = 0; k < arraysize(ontology_vector); k++) {
													if (pTmp->Get(k)->BooleanValue() == true) {
														LOGD("Obligation %d, trigger %d: purpose number %d is true", i, j, k);
														purposes[k] = '1';
													}
													else if (pTmp->Get(k)->BooleanValue() == false) {
														LOGD("Obligation %d, trigger %d: purpose number %d is false", i, j, k);
														purposes[k] = '0';
													}
													else {
														// invalid purpose vector
														LOGD("Obligation %d, trigger %d: purpose number %d is undefined", i, j, k);
														trigger.clear();
														continue;
													}
												}
											}
											else {
												LOGD("Obligation %d, trigger %d: invalid purpose parameter, wrong vector length", i, j);
												trigger.clear();
												continue;
											}
											trigger[purposeTag]=purposes;
											LOGD("Obligation %d, trigger %d: Purpose %s", i, j, purposes.c_str());
										}
										else {
											// invalid trigger: Purpose required
											LOGD("Obligation %d, trigger %d: Purpose is missing", i, j);
											trigger.clear();
											continue;
										}

										// MaxDelay
										if (triggerTmp->ToObject()->Has(String::New(maxDelayTag.c_str()))){
											v8::String::AsciiValue maxdelay(triggerTmp->ToObject()->Get(String::New(maxDelayTag.c_str())));
											trigger[maxDelayTag]=*maxdelay;
											LOGD("Obligation %d, trigger %d: MaxDelay %s", i, j, *maxdelay);
										}
										else {
											// invalid trigger: MaxDelay required
											LOGD("Obligation %d, trigger %d: MaxDelay is missing", i, j);
											trigger.clear();
											continue;
										}
									}
									// TriggerPersonalDataDeleted
									else if (strcmp(*triggerID, triggerPersonalDataDeletedTag.c_str()) == 0) {
										// MaxDelay
										if (triggerTmp->ToObject()->Has(String::New(maxDelayTag.c_str()))){
											v8::String::AsciiValue maxdelay(triggerTmp->ToObject()->Get(String::New(maxDelayTag.c_str())));
											trigger[maxDelayTag]=*maxdelay;
											LOGD("Obligation %d, trigger %d: MaxDelay %s", i, j, *maxdelay);
										}
										else {
											// invalid trigger: MaxDelay required
											LOGD("Obligation %d, trigger %d: MaxDelay is missing", i, j);
											trigger.clear();
											continue;
										}
									}
									// TriggerDataSubjectAccess
									else if (strcmp(*triggerID, triggerDataSubjectAccessTag.c_str()) == 0) {
										if (triggerTmp->ToObject()->Has(String::New(uriTag.c_str()))){
											v8::String::AsciiValue endpoint(triggerTmp->ToObject()->Get(String::New(uriTag.c_str())));
											trigger[uriTag]=*endpoint;
											LOGD("Obligation %d, trigger %d: Endpoint %s", i, j, *endpoint);
										}
										else {
											// invalid trigger: Endpoint required
											LOGD("Obligation %d, trigger %d: Endpoint is missing", i, j);
											trigger.clear();
											continue;
										}
									}
									else {
										// invalid trigger: unrecognized triggerID
										LOGD("Obligation %d, trigger %d: unrecognized triggerID %s", i, j, *triggerID);
										trigger.clear();
										continue;
									}
								}
								else {
									// invalid trigger: triggerID required
									LOGD("Obligation %d, trigger %d: triggerID is missing", i, j);
									continue;
								}
								triggers.push_back(trigger);
								trigger.clear();
							}
						}
						else {
							LOGD("Invalid triggers parameter, it is not an array");
							continue;
						}
					}
					else {
						// invalid obligation: triggers required
						LOGD("Obligation %d: triggers are missing", i);
						continue;
					}

					if (action.empty() == false && triggers.empty() == false) {
						ob.action = action;
						ob.triggers = triggers;

						obs.push_back(ob);
					}
					action.clear();
					triggers.clear();
				}
			}
			else {
				LOGD("Invalid obligations parameter, it is not an array");
			}
		}

//		string widPath(".");

//		string roam("N");
//		map<string,string>* environment = new map<string,string>();
//		(*environment)["roaming"] = roam;
		
//		Request* myReq = new Request(widPath, *resource_attrs, *environment);

		map<string, string> * environment_attrs = new map<string, string>();

		if (args[0]->ToObject()->Has(String::New("environmentInfo"))) {
			v8::Local<Value> eiTmp = args[0]->ToObject()->Get(String::New("environmentInfo"));
			/*
			if (eiTmp->ToObject()->Has(String::New("roaming"))) {
				v8::String::AsciiValue roaming(eiTmp->ToObject()->Get(String::New("roaming")));
				(*environment_attrs)["roaming"] = *roaming;
				LOGD("Parameter roaming : %s", *roaming);
			}
			if (eiTmp->ToObject()->Has(String::New("bearerType"))) {
				v8::String::AsciiValue bearerType(eiTmp->ToObject()->Get(String::New("bearerType")));
				(*environment_attrs)["bearer-type"] = *bearerType;
				LOGD("Parameter bearer-type : %s", *bearerType);
			}
			*/
			if (eiTmp->ToObject()->Has(String::New("profile"))) {
				v8::String::AsciiValue profile(eiTmp->ToObject()->Get(String::New("profile")));
				(*environment_attrs)["profile"] = *profile;
				LOGD("Parameter profile : %s", *profile);
			}
			if (eiTmp->ToObject()->Has(String::New("timemin"))) {
				v8::String::AsciiValue timemin(eiTmp->ToObject()->Get(String::New("timemin")));
				(*environment_attrs)["timemin"] = *timemin;
				LOGD("Parameter timemin : %s", *timemin);
			}
			if (eiTmp->ToObject()->Has(String::New("days-of-week"))) {
				v8::String::AsciiValue daysofweek(eiTmp->ToObject()->Get(String::New("days-of-week")));
				(*environment_attrs)["days-of-week"] = *daysofweek;
				LOGD("Parameter daysofweek : %s", *daysofweek);
			}
			if (eiTmp->ToObject()->Has(String::New("days-of-month"))) {
				v8::String::AsciiValue daysofmonth(eiTmp->ToObject()->Get(String::New("days-of-month")));
				(*environment_attrs)["days-of-month"] = *daysofmonth;
				LOGD("Parameter daysofmonth : %s", *daysofmonth);
			}
		}

		if( args.Length()>1 && args[1]->IsObject()){
				Request* myReq = new Request(*subject_attrs, *resource_attrs, purpose, obs, *environment_attrs);
				string psd;
				Effect myEff = pmtmp->pminst->checkRequest(myReq, psd);
				LOGD("[pm.cc]PATH: %s", psd.c_str());
				args[1]->ToObject()->Set(String::New("path"), String::New(psd.c_str()));
				//enum Effect {PERMIT, DENY, PROMPT_ONESHOT, PROMPT_SESSION, PROMPT_BLANKET, UNDETERMINED, INAPPLICABLE};
	
				Local<Integer> result = Integer::New(myEff);
			
				return scope.Close(result);
		}
		else{
			Request* myReq = new Request(*subject_attrs, *resource_attrs, purpose, obs, *environment_attrs);
		
			Effect myEff = pmtmp->pminst->checkRequest(myReq);

			//enum Effect {PERMIT, DENY, PROMPT_ONESHOT, PROMPT_SESSION, PROMPT_BLANKET, UNDETERMINED, INAPPLICABLE};

			Local<Integer> result = Integer::New(myEff);
		
			return scope.Close(result);
		}
	}

	
	
	
	static Handle<Value> ReloadPolicy(const Arguments& args)  {
		HandleScope scope;

		PolicyManagerInt* pmtmp = ObjectWrap::Unwrap<PolicyManagerInt>(args.This());

		LOGD("ReloadPolicy - file is %s", pmtmp->policyFileName.c_str());
		//TODO: Reload policy file
		delete pmtmp->pminst;

		map<string, vector<string>*> *pip = new map<string, vector<string>*>();
		(*pip)["http://webinos.org/subject/id/PZ-Owner"] = new vector<string>();
		(*pip)["http://webinos.org/subject/id/known"] = new vector<string>();

		if (args.Length() > 0) {
			if (!args[0]->IsObject()) {
				LOGD("Wrong parameter type");
				return ThrowException(Exception::TypeError(String::New("Bad type argument")));
			}
			if (args[0]->ToObject()->Has(String::New("http://webinos.org/subject/id/PZ-Owner"))) {
				v8::String::AsciiValue ownerId(args[0]->ToObject()->Get(String::New("http://webinos.org/subject/id/PZ-Owner")));
				(*pip)["http://webinos.org/subject/id/PZ-Owner"]->push_back(*ownerId);
			}

			if (args[0]->ToObject()->Has(String::New("http://webinos.org/subject/id/known"))) {
				v8::Local<Array> knownList = v8::Local<Array>::Cast(args[0]->ToObject()->Get(String::New("http://webinos.org/subject/id/known")));
				unsigned int i = 0;
				for (i=0; i < knownList->Length(); i++);
				{
					v8::String::AsciiValue knownId(knownList->Get(i)->ToString());
					(*pip)["http://webinos.org/subject/id/known"]->push_back(*knownId);
				}
			}

			pmtmp->pminst = new PolicyManager(pmtmp->policyFileName, pip);
		}
		else {
			LOGD("Missing argument");
			return ThrowException(Exception::TypeError(String::New("Missing argument")));
		}

		Local<Integer> result = Integer::New(0);
		
		return scope.Close(result);
	}
	
	
	static Handle<Value> GetPolicyFilename(const Arguments& args)  {
		HandleScope scope;

		PolicyManagerInt* pmtmp = ObjectWrap::Unwrap<PolicyManagerInt>(args.This());

		Local<String> result = String::New(pmtmp->policyFileName.c_str());
		
		return scope.Close(result);
	}
	
	
};

Persistent<FunctionTemplate> PolicyManagerInt::s_ct;

extern "C" {
	static void init (Handle<Object> target)  {
		PolicyManagerInt::Init(target);  
	}
	NODE_MODULE(pm, init);
} 

