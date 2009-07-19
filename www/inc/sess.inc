<?php

/*

MySQL MyISAM session handler with db-only session locking

credits to people on this page:
http://www.zend.com/zend/spotlight/code-gallery-wade8.php
including the Locking thread

You have to create a MySQL table

CREATE TABLE sessions (
   ses_id varchar(32) NOT NULL default '',
   ses_time int(11) NOT NULL default '0',
   ses_start int(11) NOT NULL default '0',
   ses_locked char(1) NOT NULL default '',
   ses_value text NOT NULL default '',
   PRIMARY KEY (ses_id)
) TYPE=MyISAM;

If you're only running one sessions database
and you can ensure that the serialized data
will never be more than 255 characters, use
TYPE=HEAP (or ENGINE=MEMORY for MyQSL 4.1+)
instead for a speed boost (db data is kept
only in memory). ses_value would need to be
changed from a type of text to a type of char(255).
i.e. ses_value char(255) NOT NULL default '',

*/

$ses_class=new session();
session_set_save_handler(array(&$ses_class,'_open'),array(&$ses_class,'_close'),array(&$ses_class,'_read'),array(&$ses_class,'_write'),array(&$ses_class,'_destroy'),array(&$ses_class,'_gc'));
session_start();

class session{
  var $ses_table='sessions';
  var $db_host='localhost';
  var $db_user='root';
  var $db_pass='rootpass';
  var $db_name='game';

  function db_connect(){
    $mysql_connect=@mysql_pconnect($this->db_host,$this->db_user,$this->db_pass);
    mysql_query("SET NAMES 'utf8'");
    $mysql_db=@mysql_select_db($this->db_name);
    if(!$mysql_connect||!$mysql_db)return false;
    else return true;
  }

  function _open($path,$name){
  	if(false==$this->db_connect())return false; // database error
  	if(session_id()==''){
      $sessionid=session_id(md5(uniqid(mt_rand(0,999999),true)));
    }else $sessionid=@mysql_real_escape_string(session_id());
  	$sessionid=@mysql_real_escape_string(session_id());
  	$query="SELECT ses_locked FROM ".$this->ses_table." WHERE ses_id='$sessionid'";
  	while(true){ // loop until we get a lock (or a database error)
    	$result=@mysql_query($query);
    	if(is_resource($result)){
    		if(1==@mysql_num_rows($result)){
    			list($locked)=@mysql_fetch_row($result);
    			if($locked==''){
    				$query2="UPDATE ".$this->ses_table." SET ses_locked='Y' WHERE ses_id='$sessionid' AND ses_locked=''";
    				@mysql_query($query2);
    				if(@mysql_affected_rows())break; // update succeeded, session is locked
    			}
    		}else{
    			$mytime=time();
    			$query2="INSERT INTO ".$this->ses_table." (ses_id, ses_time, ses_start, ses_locked, ses_value) VALUES ('$sessionid','$mytime','$mytime','Y','')";
    			@mysql_query($query2);
    			if(@mysql_affected_rows())break; // insert succeeded, session is locked
    		}
    	}else return false; //database error
    	usleep(25000);
    }
    return true;
  }

  function _close(){
  	$sessionid=@mysql_real_escape_string(session_id());
    $this->_gc(0);
    $result=@mysql_query("UPDATE ".$this->ses_table." SET ses_locked='' WHERE ses_id='$sessionid'");
    if(@mysql_affected_rows())return true;
    return false;
  }

  function _read($ses_id){
  	$ses_id=@mysql_real_escape_string($ses_id);
  	$result=@mysql_query("SELECT ses_value FROM ".$this->ses_table." WHERE ses_id='$ses_id'");
  	if(is_resource($result)){
      if(1==@mysql_num_rows($result)){
        list($value)=@mysql_fetch_row($result);
        return $value;
      }else return '';
    }else return '';
  }

  function _write($ses_id,$data){
  	$ses_id=@mysql_real_escape_string($ses_id);
  	$data=@mysql_real_escape_string($data);
  	$mytime=time();
  	@mysql_query("UPDATE ".$this->ses_table." SET ses_time='$mytime',ses_value='$data' WHERE ses_id='$ses_id'");
    if(@mysql_affected_rows())return true;
    return false;
  }

  function _destroy($ses_id){
  	$ses_id=@mysql_real_escape_string($ses_id);
    @mysql_query("DELETE FROM ".$this->ses_table." WHERE ses_id='$ses_id'");
    if(@mysql_affected_rows())return true;
    return false;
  }

  function _gc($life){
    $ses_life=strtotime('-10 minutes');
    @mysql_query("DELETE FROM ".$this->ses_table." WHERE ses_time<$ses_life");
    if(@mysql_affected_rows())return true;
    return false;
  }
}
?>
