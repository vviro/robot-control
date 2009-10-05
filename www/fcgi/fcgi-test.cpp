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
#include <boost/algorithm/string/split.hpp>
//#include <boost/numeric/ublas/vector.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

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
//	using espace boost::lexical_cast;
//	using namespace boost::numeric::ublas;

	using namespace std;
	using namespace cgicc;

	using namespace XmlRpc;
	using namespace rtaixml_domain;


	const unsigned long QUEUE_SIZE     = 1000L;
	const unsigned long TOTAL_ELEMENTS = QUEUE_SIZE * 1000L;

	enum server_state{
		CONNECTED = 0x01,
		SENDDATA = 0x02,
		PLOTING = 0x04,
		UNDEFINED = 0x0f
	};

	enum error_info{
		INVALID_PARAMETER = 0x01,
		TASK_IS_NOT_RUNNING = 0x0f
	};

	struct scope_t{
		int id;
		string name; 
		unsigned int length;
		unsigned long long seq;
	        std::vector<std::string> * signals;
	};

	struct led_t{
		unsigned int id;
		string name; 
		unsigned int length;
		unsigned long long seq;
	        std::vector<bool> * signals;
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

		int nscope;
		int nled;
		int nscope_signal;
		int nled_signal;
		std::vector<struct scope_t> * sim_scope;
		std::vector<struct led_t> * sim_led;
	
		boost::interprocess::interprocess_semaphore *sem;

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

	*state.log << "connecting to task ... " << std::endl;
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
		*state.log << "connection ko " << std::endl;
	} else {
		std::cout << "Error calling 'Connection_Request'\n\n";
		return false;
	}
	return true;
}

bool init_data_stream (string connection_id, FCgiIO * IO ) 
{
	*state.log << "init_data_stream " << std::endl;
	XmlRpcValue noArgs, resultM;

	for (int i = 0; i < state.nscope; i++){
	noArgs.clear();
	noArgs[0] = connection_id;
	noArgs[1] = i;
	if (state.connection_task->execute("Start_Data", noArgs, resultM)) {

	*state.log << "init_scope_stream " << i << " finished " << std::endl;
		*IO << "<Start_Data>" << resultM.toXml() << "</Start_Data>\n";
	}
	}

	return true;
	
}

bool get_data_structure( string server, string connection_id, FCgiIO * IO ) 
{
try {
	XmlRpcValue noArgs, resultM, resultT, result;
	int i;

	stringstream ss, ss2;
	string s;

	std::vector<std::string> strs, strs2, strs3;
	state.sim_scope = new std::vector<struct scope_t>();
	state.sim_led = new std::vector<struct led_t>();
	struct scope_t a_scope;

	XmlRpcClient * ct = new XmlRpcClient(server.c_str(), 29501);
	state.connection_task = ct;
	noArgs.clear();
	noArgs[0] = connection_id;

	if (ct->execute("tConnect", noArgs, resultM)) {
		state.connection_task = ct;		
		*IO << "<connectioninfo>" << resultM.toXml() << "</connectioninfo>\n";
		*state.log << "Successfully calling 'tConnect'\n\n";
	}
	else {
		*state.log << "Error calling 'tConnect'\n\n";
		return false;
	}

	if (ct->execute("Get_Signal_Structure", noArgs, resultT)) {
		*state.log << "Successfully calling 'Get_Signal_Structure'\n\n";
		*IO << "<signalstructure>" << resultT.toXml() << "</signalstructure>\n";
		state.nscope = resultT.size() - 1;
		for (i = 1; i <= state.nscope ; i++){
	                ss << resultT[i] << std::endl;
			s = ss.str();
			boost::split(strs, s, boost::is_any_of(","));
			boost::split(strs2, strs[1], boost::is_any_of(":"));
                        boost::split(strs3, strs[2], boost::is_any_of(":"));
			
			state.nscope_signal += lexical_cast<int>(strs2[1]);


			ss2 << strs3[1];
			s = ss2.str();
			s.resize(s.size() - 2);
		//	state.signal_names->push_back(s);
			*state.log << strs2[1] << " - " << s << "\n";
			ss.str("");
			ss2.str("");

			
			a_scope.id = i - 1;
			a_scope.name = s;
			a_scope.length = lexical_cast<int>(strs2[1]);
			std::vector<std::string>* a_signal_slot = new std::vector<std::string>(a_scope.length);
			a_scope.signals = a_signal_slot;
			
			state.sim_scope->push_back(a_scope);

			*state.log << "sim_scopes: " << (*state.sim_scope)[i-1].id << "-" << (*state.sim_scope)[i-1].name <<"-" << (*state.sim_scope)[i-1].length <<     std::endl;
		}

		*state.log << "Number of scopes :" << state.nscope << " Number of scope signals: " << state.nscope_signal << std::endl;
		//state.signal_names = new std::vector<std::string>(state.nscope_signal);
	}
	else {
		*state.log << "Error calling 'Get_Signal_Structure'\n\n";
		return false;
  	}

	if (ct->execute("Get_Digital_Structure", noArgs, resultT)) {
		*IO << "<digitalstructure>" << resultT.toXml() << "</digitalstructure>\n";	
		state.nled = resultT.size() - 1;
		
		for (i = 1; i <= state.nled ; i++){
	                ss << resultT[i] << std::endl;
			s = ss.str();
			boost::split(strs, s, boost::is_any_of(","));
			boost::split(strs2, strs[1], boost::is_any_of(":"));
			
			state.nled_signal += boost::lexical_cast<int>(strs2[1]);
		}
		*state.log << "Number of leds :" << state.nled << " Number of led signals: " << state.nled_signal << std::endl;
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
	int i;

	string error;
	form_iterator action = CGI->getElement("action");

	//*state.log << "ACTION: " << string(**action) << std::endl;
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
				if (ret){
					*IO << "<success>" << 1 << "</success>\n";
					*state.log << "connection successful: " << ret << "\n" <<  std::endl;
					//init_data_stream( state.connection_id, IO );
					bool s2 = get_data_structure( string(**node), state.connection_id, IO );
				}
				else {
					*IO << "<success>" << 0 << "</success>\n";
					*state.log << "connection failed: " << ret << "\n" <<  std::endl;
				}

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
				//init_data_stream( state.connection_id, IO );
				//_v();_v();_v();
			
				if (state.state & SENDDATA){

				state.sem->post();

				++state.t;
/*
				*IO << "<params>" << 

				    "<param>" << 
   	                                "<point><x>"<< state.t <<"</x><y>"<< (*(*state.sim_scope)[0].signals)[0] <<"</y></point>" <<
                                        "<name>" << (*state.sim_scope)[0].name << "</name>" <<
				    "</param>" <<
				    "<param>" << 
   	                                "<point><x>"<< state.t <<"</x><y>"<< (*(*state.sim_scope)[1].signals)[0] <<"</y></point>" <<
                                        "<name>" << (*state.sim_scope)[1].name << "</name>" <<
				    "</param>" <<
				    "<param>" << 
   	                                "<point><x>"<< state.t <<"</x><y>"<< (*(*state.sim_scope)[2].signals)[0] <<"</y></point>" <<
                                        "<name>" << (*state.sim_scope)[2].name << "</name>" <<
			//		"<point><x>"<< state.t <<"</x><y>"<< (rand()%100)+1 <<"</y></point>" << 
			//		"<name>U1</name>" << 
				    "</param>" <<
				"</params>\n";
*/

					*IO << "<params>";
					for (i = 0; i < state.nscope_signal; i++){
     	                                   *IO << "<param>" <<
        	                               "<point><x>"<< state.t <<"</x><y>"<< (*(*state.sim_scope)[i].signals)[0] <<"</y></point>" <<
                	                        "<name>" << (*state.sim_scope)[i].name << "</name>" <<
                                    		"</param>";
					}
					*IO << "</params>\n" << std::endl;

				}
			}
		
		} else if (string(**action) == "add_plot") {
			*state.log << "!state.state & SENDDATA: " << !(state.state & SENDDATA)  << std::endl;
			if(!(state.state & SENDDATA)){
				init_data_stream( state.connection_id, IO );
			}
			*state.log << "signal ploting ..." << std::endl;
			if (!(state.state & PLOTING) ){
				state.sem->post();
			}

			state.state |= PLOTING;

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

	
	state.sem->wait();

	*state.log << "Thread RECV: actived \n" << std::endl;

	/* connect to task */
	int fd; int i;
	int id;
	unsigned long long seq;

	struct sockaddr_in serverip;
	char data[25*100000];
	bzero(data, sizeof(data));

	std::vector<std::string> strs, strs2;

	fd = socket(AF_INET, SOCK_STREAM, 0);
//	fcntl(fd, F_SETFL, O_NONBLOCK);

	bzero(&serverip, sizeof(serverip));
	serverip.sin_family = AF_INET;
	serverip.sin_port = htons(29502);
	serverip.sin_addr.s_addr = inet_addr("10.0.9.21");

	ret = connect(fd, (struct sockaddr *)&serverip, sizeof(serverip));
	*state.log << "Conneting ... : " << ret << std::endl;

	//state.sem->wait();
	state.state |= SENDDATA;


	while (1){
		/* clean buffer */
		bzero(data, sizeof(data));

		strs.clear();
		strs2.clear();

		state.sem->wait();

		*state.log << "========================= Thread RECV: " << counter++ << " ====================== \n" << std::endl;
                ret = read(fd, data, sizeof(data));

		boost::split(strs, data, boost::is_any_of("\n"));

		*state.log << "socket: read: " << ret << "  errno: " << errno << std::endl;
                *state.log << "Received: \n" <<  data << std::endl;
                *state.log << "Received:  " << strs.size() - 1 << " data" << "  last: " << strs[strs.size() - 2] << "\n" << std::endl;


//		for ( i = 0; i < strs.size() - 5; i++){
//	                *state.log << "====" << strs[i] << "\n" << std::endl;
//		}
	

//	state.sem->wait();

		
		boost::split(strs2, strs[strs.size() - 2], boost::is_any_of(" "));

                *state.log << "Received(last) Tokens: " << strs2[0] << "-" << strs2[1] << "-" << strs2[2]  << std::endl;

		id = lexical_cast<int>(strs2[0]);
		seq = lexical_cast<unsigned long long>(strs2[1]);

 		(*state.sim_scope)[id].seq = seq;
		(*(*state.sim_scope)[id].signals)[0] = string(lexical_cast<string>(1000*lexical_cast<double>(strs2[2])));

		*state.log << ">>>>>>>>>>>>>>>>" << "id: "<< id << "  seq: " << seq << "   value: "  << lexical_cast<double>(strs2[2]) << std::endl;

	
	}
}

int init()
{
	state.state = 0;
	state.t = 0;
	state.nscope = 0;
	state.nled = 0;
	state.nscope_signal = 0;
	state.nled_signal = 0;
}

int main(int /*argc*/, const char **/*argv*/, char **/*envp*/)
{
	/* init globale variables */
	init();

	/* init log */
 	remove_all("log");
	create_directory("log");
   	state.log = new ofstream("log/log");

	/* init sem */
	//semid = semget((key_t)1234, 1, 0666 | IPC_CREAT);
	//*state.log << "SEMID: " << semid << "  errno: " << errno <<"\n" << std::endl;

	/*
	union semun sem_union;
	sem_union.val = 1;
	if (semctl(semid, 0, SETVAL, sem_union) == -1){
		*state.log << "semctl error\n" << std::endl;
	}else{
		*state.log << "semctl ok\n" << std::endl;
	}*/

	state.sem = new boost::interprocess::interprocess_semaphore(0);

	/* init thread */
	pthread_t thread;
	pthread_create(&thread, NULL, receiver, NULL);

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
