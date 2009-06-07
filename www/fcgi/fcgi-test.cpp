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

#include <unistd.h>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>

#include <xmlrpc++/XmlRpc.h>
#include <rtaixml/Socket.h>
#include <rtaixml/SocketException.h>

#include "cgicc/HTTPResponseHeader.h"
#include "cgicc/HTMLClasses.h"
#include "FCgiIO.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using boost::lexical_cast;

using namespace std;
using namespace cgicc;

using namespace XmlRpc;
using namespace rtaixml_domain;

struct state_t
{
	bool connected;
	string connection_id;
} state;

bool connect(string server, int port)
{
	// port should be 29500

	XmlRpcClient cm(server.c_str(), port);
	XmlRpcValue noArgs, resultM, resultT, result;

	//std::cout << "connecting to manager   ";
	noArgs[0]="test";
	noArgs[1]="Master";
	if (cm.execute("Connection_Request", noArgs, resultM))
	{
		state.connection_id = string(resultM["id_session"]);
		state.connected = true;
	}
	else
	{
		std::cout << "Error calling 'Connection_Request'\n\n";
		return false;
	}
	return true;
}

bool get_data_structure( string server, string connection_id, FCgiIO * IO ) 
{
	XmlRpcValue noArgs, resultM, resultT, result;

	XmlRpcClient ct(server.c_str(), 29501);
	noArgs.clear();
	noArgs[0] = connection_id;
	if (ct.execute("tConnect", noArgs, resultT))
		*IO << "<connection_info>" << resultT << "</connection_info>" << "\n";
	else {
		std::cout << "Error calling 'tConnect'\n\n";
		return false;
	}
	if (ct.execute("Get_Signal_Structure", noArgs, resultT))
		*IO << "<structure>" << resultT << "</structure>\n";
	 
	else {
		std::cout << "Error calling 'tConnect'\n\n";
		return false;
  	}
	return true;
}

void dispatch(Cgicc * CGI, FCgiIO * IO )
{
	string error;
	form_iterator action = CGI->getElement("action");
	if(action != CGI->getElements().end())
	{
		if ( string(**action) == "connect")
		{
			if (!state.connected)
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

				bool s = connect( (**node).c_str(), lexical_cast<int>((**port).c_str()) );
				if (s)
					*IO << "<success>" << 1 << "</success>" << endl;
				else
					*IO << "<success>" << 0 << "</success>" << endl;

				if (s)
					bool s2 = get_data_structure( string(**node), state.connection_id, IO );
			}
			else
			{
				*IO << "<success>" << 1 << "</success>" << endl;
			}
		} else
		{
			error += "wrong arguments\n";
		}
	}
	if (error.length() > 0) {
		*IO << "<error>" << error << "</error>" << endl;		
	}

}


void print_header( FCgiIO * IO )
{
	// Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info

	*IO << HTTPResponseHeader("HTTP/1.1", 200, "OK")
		.addHeader("Content-Language", "en")
		.addHeader("Content-Type", "text/xml");

//	*IO << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
//	*IO << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">" << endl;
	*IO << "<r>\n";
}


void print_footer( FCgiIO* IO )
{
	*IO << "</r>" << endl;
}


int
main(int /*argc*/,
const char **/*argv*/,
char **/*envp*/)
{
	unsigned count = 0;
	state.connected = false;

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
