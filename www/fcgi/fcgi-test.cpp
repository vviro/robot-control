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

#include <XmlRpc/XmlRpc.h>
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

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/progress.hpp>
#include <boost/bind.hpp>
#include <deque>
#include <list>
#include <string>

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <stropts.h>
#include <sys/uio.h>
#include <unistd.h>

static int semid;


#include <boost/filesystem.hpp>  
	using namespace boost::filesystem;  


	using namespace boost;
	using boost::lexical_cast;

	using namespace std;
	using namespace cgicc;

	using namespace XmlRpc;
	using namespace rtaixml_domain;


	const unsigned long QUEUE_SIZE     = 1000L;
	const unsigned long TOTAL_ELEMENTS = QUEUE_SIZE * 1000L;

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
		int t;
		ofstream * log;
	} state;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

static int _v(void)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;

	if (semop(semid, &sem_b, 1) == -1){
		return -1;
	}
	return 0;
}

static int _p(void)
{
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(semid, &sem_b, 1) == -1){
		return -1;
	}
	return 0;
}


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
	catch(std::exception &e)
	{
		cout << e.what() << endl;
		ret += "\n std::exception: " + string(e.what());
		return ret;
	}
}

bool connect(string server, int port)
{
	// port should be 29500

	XmlRpcClient * cm = new XmlRpcClient(server.c_str(), port);
	XmlRpcValue noArgs, resultM, resultT, result;

	//std::cout << "connecting to manager   ";
	noArgs[0]="test";
	noArgs[1]="Master";
	if (cm->execute("Connection_Request", noArgs, resultM))	{
		state.connection_id = string(resultM["id_session"]);
		state.state = state.state | CONNECTED ;
		state.noArgs = new XmlRpcValue(noArgs);
		state.connection_server = cm;
	} else {
		std::cout << "Error calling 'Connection_Request'\n\n";
		return false;
	}
	return true;
}
i

bool init_data_stream (string connection_id, FCgiIO * IO ) 
{
//	*state.log << "init_data_stream " << std::endl;
try {
	XmlRpcValue noArgs, resultM;

	XmlRpcClient * ct = state.connection_server;
	noArgs.clear();
	noArgs[0] = connection_id;
	noArgs[1] = 0;
	if (ct->execute("Start_Data", noArgs, resultM)) {
		state.connection_task = ct;		
		*IO << "<Start_Data>" << resultM.toXml() << "</Start_Data>\n";
	}

	return true;
} catch (std::exception &e) {
	*IO << e.what() << endl;
}
return true;
}

bool get_data_structure( string server, string connection_id, FCgiIO * IO ) 
{
try {
	XmlRpcValue noArgs, resultM, resultT, result;

	XmlRpcClient * ct = new XmlRpcClient(server.c_str(), 29501);
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

	if (ct->execute("Get_Digital_Structure", noArgs, resultT)) {
		*IO << "<digitalstructure>" << resultT.toXml() << "</digitalstructure>\n";
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
} catch (std::exception &e) {
	*IO << e.what() << endl;
}
	return true;
}

void dispatch(Cgicc * CGI, FCgiIO * IO )
{
	int ret;

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

		} else if (string(**action) == "get_data_stream") { /////// for charts
			if ( state.state & CONNECTED ){
				//*state.log << "get_data_stream: binding to 29502\n" << std::endl;
				init_data_stream( state.connection_id, IO );
				_v();_v();_v();


			}
			++state.t;
			*IO << "<params>" << 
				    "<param>" << 
					"<point><x>"<< state.t <<"</x><y>"<< (rand()%100)+1 <<"</y></point>" << 
					"<name>Winkel Scope</name>" << 
				    "</param>" <<
				    "<param>" << 
					"<point><x>"<< state.t <<"</x><y>"<< (rand()%100)+1 <<"</y></point>" << 
					"<name>Controller Switcher</name>" << 
				    "</param>" <<
				    "<param>" << 
					"<point><x>"<< state.t <<"</x><y>"<< (rand()%100)+1 <<"</y></point>" << 
					"<name>Wikel Velocity Scope</name>" << 
				    "</param>" <<
				"</params>\n";
		} else if (string(**action) == "add_plot") {
			//CGI->getElement("varname");

			*state.log << "signal ploting ...\n" << std::endl;
			ret = _v();
			ret = _v();
			ret = _v();

			*state.log << "_v returnd: " << ret << "\n" <<  std::endl;
			
		} else if (string(**action) == "remove_plot") {
		
		}	
		else
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

double test;

void *receiver(void *)
{
	*state.log << "Thread RECV: started \n" << std::endl;

	int counter = 0; 
	int ret = -2;

	
	/* init socket */
	ret = _p();
	*state.log << "_p returnd: " << ret << "\n" <<  std::endl;

	/*

	rtaixml_domain::Socket data_sock;
        std::string data_from_server;

        data_sock.create();

        if (!data_sock.connect("10.0.9.21", 29502)){
                     *state.log << "Error by connecting to the data socket\n";
	} else {
           	*state.log << "socket initialization: ok\n";
	}


	while (1){
		ret = _p();
		*state.log << "_p returnd: " << ret << "\n" <<  std::endl;
		*state.log << "Got seed: " << counter ++ << "\n" << std::endl;
		ret = data_sock.recv(data_from_server);
		*state.log << "Received:  " <<  data_from_server << "\n" << std::endl;
	}
	*/


	int fd;
	struct sockaddr_in serverip;
	char data[4096];
	bzero(data, sizeof(data));

	fd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK);

	bzero(&serverip, sizeof(serverip));
	serverip.sin_family = AF_INET;
	serverip.sin_port = htons(29502);
	serverip.sin_addr.s_addr = inet_addr("10.0.9.21");

	ret = connect(fd, (struct sockaddr *)&serverip, sizeof(serverip));

	*state.log << "Conneting ... : " << ret << std::endl;
	ret = _p();
	while (1){
		 //ret = _p();
                *state.log << "_p returnd: " << ret << "\n" <<  std::endl;
                *state.log << "Got seed: " << counter ++ << "\n" << std::endl;
                ret = read(fd, data, sizeof(data));
		*state.log << "read: " << ret << "  errno: " << errno << std::endl;
                *state.log << "Received:  " <<  data << "\n" << std::endl;

	}

}

int main(int /*argc*/, const char **/*argv*/, char **/*envp*/)
{
	/* init log */
 	remove_all("log");
	create_directory("log");
   	state.log = new ofstream("log/log");

	/* init sem */
	semid = semget((key_t)1234, 1, 0666 | IPC_CREAT);
	*state.log << "SEMID: " << semid << "  errno: " << errno <<"\n" << std::endl;

	union semun sem_union;
	sem_union.val = 1;
	if (semctl(semid, 0, SETVAL, sem_union) == -1){
		*state.log << "semctl error\n" << std::endl;
	}else{
		*state.log << "semctl ok\n" << std::endl;
	}

	/* init thread */
	pthread_t thread;
	pthread_create(&thread, NULL, receiver, NULL);

	/* init gloable variables */
	state.state = 0;
	state.t = 0;

	/* init fast cgi */
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
		catch(const std::exception& e)
		{
			std::cout << e.what() << endl;
		}

		FCGX_Finish_r(&request);
	}

	return 0;
}
