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


(function () {
	"use strict";

	var os = require('os');
	var promptLib = require('../src/promptMan/promptManager.js');
	var dslib = require('./decisionstorage.js');
	var JSV = require('JSV').JSV;
	var fs = require('fs');
	var xml2js = require('xml2js');
	var schema = require('./schema.json');
	var env = JSV.createEnvironment("json-schema-draft-03");
	var xmlParser = new xml2js.Parser(xml2js.defaults["0.2"]);
	var bridge = null;
	var pmCore = null;
	var pmNativeLib = null;
	var promptMan = null;
	var policyFile = null;
	var decisionStorage = null;
	var promptCallbacks = {};

	var policyManager = function(policyFilename) {
		var self = this;
		// Load the native module
		try {
			pmNativeLib = require('pm');
		} catch (err) {
			console.log("Warning! Policy manager could not be loaded");
		}
		// Load the prompt manager
		if (os.platform()==='android') {
			bridge = require('bridge');
			promptMan = bridge.load('org.webinos.impl.PromptImpl', self);
		}
		else if (os.platform()==='win32') {
			promptMan = require('promptMan');
		}
		else {
			promptMan = new promptLib.promptManager();
		}
		//Policy file location
		policyFile = policyFilename;

		self.isAWellFormedPolicyFile(policyFile
			, function () {
				pmCore = new pmNativeLib.PolicyManagerInt(policyFile);
				//Loads decision storage module
				decisionStorage = new dslib.decisionStorage(policyFile);
				console.log("Policy file loaded");
			}
			, function () {
				console.log("Policy file is not valid");
			}
		);
	};

	policyManager.prototype.getPolicyFilePath = function() {
		return policyFile;
	}

	policyManager.prototype.enforceRequest = function(request, sessionId, noprompt, cb) {
		if (!pmCore) {
			console.log("Invalid policy file: request denied")
			cb(false);
			return;
		}
		var res = pmCore.enforceRequest(request);
		var promptcheck = true;
		if (arguments.length == 3) {
			if (noprompt == true)
				promptcheck = false;
		}

		if(res>1 && res<5) {
			// if there is a promptMan then show a message
			if (promptMan && decisionStorage && promptcheck) {
				var storedDecision = decisionStorage.checkDecision(request, sessionId);
				if(storedDecision == 0 || storedDecision == 1) {
					res = storedDecision;
				}
				else {
					// Set prompt choices based on the following:
					// 0 = "Deny always";
					// 1 = "Deny for this session";
					// 2 = "Deny this time";
					// 3 = "Allow this time";
					// 4 = "Allow for this session";
					// 5 = "Allow always";

					var choices;
					if(res == 2) {
						//Prompt oneshot
						choices = "0|2|3";
					}
					else if(res==3) {
						//Prompt session
						choices = "0|1|2|3|4";
					}
					else {
						//Prompt blanket
						choices = "0|1|2|3|4|5";
					}

					doPrompt(request, sessionId, cb, choices);
				}
			}
		}
		else {
			console.log("Policy Manager enforce request: "+JSON.stringify(request)+" - result is "+res);
			cb(res);
		}
	};

	policyManager.prototype.reloadPolicy = function () {
		self.isAWellFormedPolicyFile(policyFile
			, function () {
				pmCore.reloadPolicy();
			}
			, function () {
				console.log("Policy file is not valid");
			}
		);
	};

	policyManager.prototype.isAWellFormedPolicyFile = function (policyFilename, successCB, errorCB) {
		var data = fs.readFileSync(policyFilename);

		xmlParser.parseString(data, function (err, jsonData) {
			if (!err) {
				(env.validate(jsonData, schema).errors.length === 0) ? successCB() : errorCB();
			} else {
				errorCB();
			}
		});
	}

	function doPrompt(request, sessionId, cb, choiceList) {
		// ****
		// Create a unique token and store it with the callback 'cb'
		// Call the dashboard to display the prompt.
		// Dashboard will call back into the pzp with the token and the response to the prompt (hacked with http request at the moment)
		// Lookup callback using token and call it with the response.
		// If timeout occurs, prompt is stored in list (to-do), and callback called with 'denied'.
		// ****

		var promptTimeout = 20000;

		// Create a token to store details of this prompt.
		var uuid = require('node-uuid');
		var token = uuid.v1();

		// Store the callback with the token.
		promptCallbacks[token] = {
			callback: cb,
			request: request,
			sessionId: sessionId
		};

		// Create a timeout - deny permission after 20 secs.
		setTimeout(function() {
			// Prompt has timed out - check that the user didn't respond in the meantime.
			if (promptCallbacks.hasOwnProperty(token)) {
				// Callback still exists => no user response received.
				var cb = promptCallbacks[token].callback;
				delete promptCallbacks[token];
				// Callback with 'denied'
				cb(false);
			}
		}, promptTimeout);

		// Use applauncher to launch dashboard, passing the token as a launch argument.
		var appLauncher = require("webinos-api-applauncher/lib/applauncher_impl");
		appLauncher.launchApplication("http://webinos.org/dashboard?prompt=\"" + token + "\"&choices=\"" + choiceList + "\"&user=\"" + request.subjectInfo.userId + "\"&feature=\"" +  encodeURIComponent(request.resourceInfo.apiFeature) + "\"&timeout=" + promptTimeout,
			function() {
				// Launch successful => do nothing (wait for user response or timeout).
			},
			function() {
				// Failed to launch dashboard => deny permission.
				cb(false);
			}
		);
	}

	policyManager.prototype.storePermissionResponse = function(request, sessionId, reply) {
		var permit = false;
		var storeSession = false;
		var storePermanent = false;

		// Determine the policy decision and storage required based on the response.
		switch (reply) {
			case 0:
				// Deny always.
				permit = false;
				storePermanent = true;
				break;
			case 1:
				// Deny this session.
				permit = false;
				storeSession = true;
				break;
			case 2:
				// Deny this time.
				permit = false;
				break;
			case 3:
				// Allow this time.
				permit = true;
				break;
			case 4:
				// Allow this session.
				permit = true;
				storeSession = true;
				break;
			case 5:
				// Allow always.
				permit = true;
				storePermanent = true;
				break;
			default:
				permit = false;
				break;
		}
		if (storeSession || storePermanent) {
			decisionStorage.addDecision(request, sessionId, permit ? 1 : 0, storePermanent ? 0 : 1);
		}

		return permit;
	}

	policyManager.prototype.replyPrompt = function(token, reply) {
		// Response from prompt received from user.
		// Check that the prompt hasn't timed out.
		if (promptCallbacks.hasOwnProperty(token)) {
			// Prompt still valid - get the details.
			var cb = promptCallbacks[token].callback;
			var request = promptCallbacks[token].request;
			var sessionId = promptCallbacks[token].sessionId;
			delete promptCallbacks[token];

			// Store the permission request response.
			var permit = this.storePermissionResponse(request, sessionId, reply);

			cb(permit);
		}
	}

	exports.policyManager = policyManager;

}());
