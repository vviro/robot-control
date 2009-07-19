-- MySQL dump 10.11
--
-- Host: localhost    Database: game
-- ------------------------------------------------------
-- Server version	5.0.32-Debian_7-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `games`
--

DROP TABLE IF EXISTS `games`;
CREATE TABLE `games` (
  `gameid` int(10) unsigned NOT NULL auto_increment,
  `oppid` int(10) unsigned default NULL,
  `start_phrase` varchar(200) collate utf8_unicode_ci default NULL,
  `end_phrase` varchar(200) collate utf8_unicode_ci default NULL,
  `start_time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `timelimit` int(11) default NULL,
  PRIMARY KEY  (`gameid`),
  UNIQUE KEY `oppid` (`oppid`)
) ENGINE=MyISAM AUTO_INCREMENT=123 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table `games_log`
--

DROP TABLE IF EXISTS `games_log`;
CREATE TABLE `games_log` (
  `gameid` int(11) NOT NULL,
  `uid` int(11) NOT NULL,
  `ngram` tinyint(3) unsigned default NULL,
  `position` int(11) default NULL,
  `phraseid` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table `history`
--

DROP TABLE IF EXISTS `history`;
CREATE TABLE `history` (
  `uid` int(11) NOT NULL,
  `phrase` varchar(200) collate utf8_unicode_ci default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table `messages`
--

DROP TABLE IF EXISTS `messages`;
CREATE TABLE `messages` (
  `sender_uid` int(10) unsigned NOT NULL,
  `recepient_uid` int(10) unsigned NOT NULL,
  `body` text collate utf8_unicode_ci NOT NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `messid` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`messid`)
) ENGINE=MyISAM AUTO_INCREMENT=1693 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table `opponents`
--

DROP TABLE IF EXISTS `opponents`;
CREATE TABLE `opponents` (
  `oppid` int(10) unsigned NOT NULL auto_increment,
  `time_formed` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `time_closed` timestamp NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`oppid`)
) ENGINE=MyISAM AUTO_INCREMENT=398 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table `sessions`
--

DROP TABLE IF EXISTS `sessions`;
CREATE TABLE `sessions` (
  `ses_id` varchar(32) collate utf8_unicode_ci NOT NULL default '',
  `ses_time` int(11) NOT NULL default '0',
  `ses_start` int(11) NOT NULL default '0',
  `ses_locked` char(1) collate utf8_unicode_ci NOT NULL default '',
  `ses_value` text collate utf8_unicode_ci NOT NULL,
  `uid` int(10) unsigned default NULL,
  `wannaplay` tinyint(1) default '0',
  `partner_uid` int(10) unsigned default '0',
  `time_diff` int(11) default NULL,
  PRIMARY KEY  (`ses_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*!50003 SET @OLD_SQL_MODE=@@SQL_MODE*/;
DELIMITER ;;
/*!50003 SET SESSION SQL_MODE="" */;;
/*!50003 CREATE */ /*!50017 DEFINER=`root`@`localhost` */ /*!50003 TRIGGER `clear_session` BEFORE DELETE ON `sessions` FOR EACH ROW begin
update users_opponents set active = 0 where uid = OLD.uid;
end */;;

DELIMITER ;
/*!50003 SET SESSION SQL_MODE=@OLD_SQL_MODE */;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `uid` int(10) unsigned NOT NULL auto_increment,
  `username` varchar(32) NOT NULL,
  `pass` varchar(32) character set latin1 NOT NULL,
  `active` tinyint(1) default '1',
  `name` varchar(128) default NULL,
  `email` varchar(128) default NULL,
  `time_added` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `active_till` timestamp NOT NULL default '0000-00-00 00:00:00',
  `wants_news` tinyint(1) default '0',
  `status` enum('guest','user','premium','admin') default 'user',
  PRIMARY KEY  (`uid`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM AUTO_INCREMENT=23 DEFAULT CHARSET=utf8;

--
-- Table structure for table `users_opponents`
--

DROP TABLE IF EXISTS `users_opponents`;
CREATE TABLE `users_opponents` (
  `oppid` int(11) NOT NULL,
  `uid` int(11) NOT NULL,
  `active` tinyint(1) default '1',
  UNIQUE KEY `uid` (`uid`,`oppid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2007-03-08 20:49:13
