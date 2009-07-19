/*
 * $Id: fcgi-test.cpp,v 1.5 2007/07/02 18:48:19 sebdiaz Exp $
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth
 *                       2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
 *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 */

#include <exception>
#include <iostream>
#include <sstream>

#include <unistd.h>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>

#include <XmlRpc.h>
#include <rtaixml/Socket.h>
#include <rtaixml/SocketException.h>

#include "cgicc/HTTPResponseHeader.h"
#include "cgicc/HTMLClasses.h"
#include "FCgiIO.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/smart_ptr.hpp>

using namespace boost;
using boost::lexical_cast;

using namespace std;
using namespace cgicc;

using namespace XmlRpc;
using namespace rtaixml_domain;


enum server_state{
	CONNECTED = 0x01,
	UNDEFINED = 0x0f
};

enum error_info{
	INVALID_PARAMETER = 0x01,
	TASK_IS_NOT_RUNNING = 0x0f
};

struct state_t
{
	int state;
	int error;
	string connection_id;
	XmlRpcValue * server_params;
        XmlRpcValue * noArgs;
	XmlRpcClient * connection_task;
	XmlRpcClient * connection_server;
} state;


string update_param(string param)
{
	std::string ret;

	try{	
		vector< string > lines, items; 
		vector< vector < string > > ar (10000);

	        split( lines, param, is_any_of("\n") );
		int i = 0;
		for (vector<string>::iterator it = lines.begin(); it != lines.end(); ++it) {
			vector< string > t(5);
			ret += *it + "\n";
			tokenizer<escaped_list_separator<char> > tok( *it );
			int j = 0;
			for(tokenizer<escaped_list_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg) {
				t.at(j) = string(*beg);
				++j;
			}
			ar.at(i) = t;		
			++i;
		}
		// changing server state

		XmlRpcValue sp = *state.server_params;
		sp[0] = state.connection_id;
		ret += "server_params size: " + boost::lexical_cast<string>(sp.size()) + "\n" ;

		XmlRpcValue nv;
		for (int k = 0; k < i-1; k++) {
			string blockname = ar.at(k).at(0) + "/" + ar.at(k).at(1);
			string param_name = ar.at(k).at(2);
			string value = ar.at(k).at(3);
			ret += "setting " + blockname + " to " + value + "...\n";				

			for(int h=1; h < sp.size(); ++h) {
				ret += "value " +string((sp[h])[0])+ " = " + string("1")+";\t";
				nv[0] = boost::lexical_cast<double>(value);
				if (string(sp[h][0]) == blockname & string(sp[h][1]) == param_name) {
					sp[h][4] = nv;
					ret += blockname + " (" + param_name+ ") set.\n";				
					break;
				}
			}
		}

		state.server_params = new XmlRpcValue(sp);
	
		XmlRpcValue r;
		if (state.connection_task->execute("Send_Param", *state.server_params, r)){
			stringstream ss;
			ss << "Sent parameters: " << sp << "\nGot response: "<< r;
			ret += ss.str()+"\n";
			ret += string("session_id = ") + string(sp[0]) + "\n";
			ret += string("\nsuccessfully updated parameters");
		}
		else {
		        ret = "Error calling 'tConnect'\n\n";
		}

		return ret;
	}
	catch(exception &e)
	{
		cout << e.what() << endl;
		ret += "\n exception: " + string(e.what());
		return ret;
	}
}

bool connect(string server, int port)
{
	// port should be 29500

	XmlRpcClient * cm = new XmlRpcClient(server.c_str(), port, "", false);
	XmlRpcValue noArgs, resultM, resultT, result;

	//std::cout << "connecting to manager   ";
	noArgs[0]="v2_visual";
	noArgs[1]="Master";
	if (cm->execute("Connection_Request", noArgs, resultM))
	{
		state.connection_id = string(resultM["id_session"]);
		state.state = state.state | CONNECTED ;
		state.noArgs = new XmlRpcValue(noArgs);
		state.connection_server = cm;
	} else
	{
		std::cout << "Error calling 'Connection_Request'\n\n";
		return false;
	}
	return true;
}

bool get_data_structure( string server, string connection_id, FCgiIO * IO ) 
{
try {
	XmlRpcValue noArgs, resultM, resultT, result;

	XmlRpcClient * ct = new XmlRpcClient(server.c_str(), 29501, "", false);
	noArgs.clear();
	noArgs[0] = connection_id;

	if (ct->execute("tConnect", noArgs, resultM)) {
		state.connection_task = ct;		
		*IO << "<connectioninfo>" << resultM.toXml() << "</connectioninfo>\n";
	}
	else {
		std::cout << "Error calling 'tConnect'\n\n";
		return false;
	}

	if (ct->execute("Get_Signal_Structure", noArgs, resultT)) {
		*IO << "<signalstructure>" << resultT.toXml() << "</signalstructure>\n";
	}
	else {
		std::cout << "Error calling 'tConnect'\n\n";
		return false;
  	}

	if (ct->execute("Get_Param", noArgs, resultT)) {
		state.server_params = new XmlRpcValue(resultT);
		state.noArgs = new XmlRpcValue(noArgs);
		*IO << "<parameters>" << resultT.toXml() << "</parameters>\n";
	}
} catch (exception &e) {
	*IO << e.what() << endl;
}
	return true;
}

void dispatch(Cgicc * CGI, FCgiIO * IO )
{
	bool ret;

	string error;
	form_iterator action = CGI->getElement("action");
	if(action != CGI->getElements().end())
	{
		if ( string(**action) == "connect")
		{
			if (! (state.state & CONNECTED))
			{

				form_iterator node = CGI->getElement("node");
				if(node == CGI->getElements().end())
				{
					error += "host not defined\n";
				}

				form_iterator port = CGI->getElement("port");
				if(port == CGI->getElements().end())
				{
					error += "port not defined\n";
				}

				form_iterator task = CGI->getElement("task");
				if(task != CGI->getElements().end())
				{
					//	cout << "Task Name: " << **task << endl;
				}

				ret = connect( (**node).c_str(), lexical_cast<int>((**port).c_str()) );
				if (ret)
					*IO << "<success>" << 1 << "</success>\n";
				else
					*IO << "<success>" << 0 << "</success>\n";

				if (ret)
					bool s2 = get_data_structure( string(**node), state.connection_id, IO );
			}
			else
			{
				*IO << "<success>" << 1 << "</success>\n";
			}

		} else if ( string(**action) == "update") {

			if ( state.state & CONNECTED ){

				form_iterator update_newparam = CGI->getElement("changed_params");
				if(update_newparam == CGI->getElements().end())
				{
					error += "invalid new parameters\n";
				}

				string params = update_param( string((**update_newparam).c_str()) );
				*IO << "<changed_params>" << params << "</changed_params>\n";
			} else {
				error += "not connected\n";
			}

		} else
		{
			error += "wrong arguments\n";
		}
	}
	if (error.length() > 0) {
		*IO << "<error>" << error << "</error>\n";
	}

}


void print_header( FCgiIO * IO )
{
	// Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info

	*IO << HTTPResponseHeader("HTTP/1.1", 200, "OK")
		.addHeader("Content-Language", "en")
		.addHeader("Content-Type", "text/xml");
	*IO << "<r>\n";
}


void print_footer( FCgiIO* IO )
{
	*IO << "</r>\n" << endl;
}


int main(int /*argc*/, const char **/*argv*/, char **/*envp*/)
{

	state.state = 0;

	FCGX_Request request;

	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);

	while(FCGX_Accept_r(&request) == 0)
	{

		try
		{
			FCgiIO IO(request);
			Cgicc CGI(&IO);

			print_header(&IO);

			dispatch(&CGI, &IO);

			/*
			for(const_form_iterator i = CGI.getElements().begin();
			i != CGI.getElements().end(); ++i )
			  IO << "<" << i->getName() << ">" << i->getValue() << "</" << i->getName() << ">" << endl;
			 */

			print_footer(&IO);
		}
		catch(const exception& e)
		{
			std::cout << e.what() << endl;
		}

		FCGX_Finish_r(&request);
	}

	return 0;
}
