DROP TABLE IF EXISTS `admin_logs`;
CREATE TABLE `admin_logs` (
  `id` int(11) NOT NULL auto_increment,
  `user_id` int(11) NOT NULL,
  `op` varchar(3000) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `admins`;
CREATE TABLE `admins` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(100) NOT NULL,
  `email` varchar(20) NOT NULL,
  `session` varchar(200) default NULL,
  `password` varchar(200) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `connection_logs`;
CREATE TABLE `connection_logs` (
  `id` int(11) NOT NULL auto_increment,
  `ts` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `connections` int(11) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `connections`;
CREATE TABLE `connections` (
  `id` int(11) NOT NULL auto_increment,
  `ip` varchar(20) NOT NULL,
  `connection_traffic` bigint(20) default NULL,
  `start_time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `end_time` timestamp NOT NULL default '0000-00-00 00:00:00',
  `username` varchar(200) default NULL,
  `interface` varchar(20) default NULL,
  `ppid` int(11) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `coupons`;
CREATE TABLE `coupons` (
  `id` int(11) NOT NULL auto_increment,
  `status` enum('N','U','R','F') default 'N',
  `credit` bigint(20) NOT NULL,
  `generator` varchar(2000) NOT NULL,
  `user` varchar(200) default NULL,
  `token` char(32) NOT NULL,
  `pass` char(32) NOT NULL,
  `salt` varchar(1000) default NULL,
  `ts` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `expire` int(11) NOT NULL default '30',
  `notes` varchar(1024) default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `salt_2` (`salt`),
  KEY `salt` (`salt`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `host_info`;
CREATE TABLE `host_info` (
  `host` varchar(200) default NULL,
  `host_group` int(11) default '0'
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `login`;
CREATE TABLE `login` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(16) NOT NULL,
  `password` varchar(32) NOT NULL,
  `status` int(11) NOT NULL default '0',
  `clientip` varchar(15) NOT NULL,
  `serverip` varchar(15) NOT NULL,
  `credit` bigint(20) default NULL,
  `email` varchar(50) default NULL,
  `expire_at` datetime default NULL,
  `host_group` int(11) default '0',
  `report` enum('DAILY','WEEKLY','NO') NOT NULL default 'DAILY',
  `account_state` enum('ACTIVE','DISABLED') NOT NULL default 'ACTIVE',
  `notes` varchar(1024) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `username` (`username`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `logs`;
CREATE TABLE `logs` (
  `id` int(11) NOT NULL auto_increment,
  `ts` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `event` varchar(1000) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `openvpn_connections`;
CREATE TABLE `openvpn_connections` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(100) NOT NULL,
  `ip` varchar(20) NOT NULL,
  `port` int(11) NOT NULL,
  `internalip` varchar(20) NOT NULL,
  `traffic` int(11) NOT NULL,
  `since` int(11) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `payment_transactions`;
CREATE TABLE `payment_transactions` (
  `id` int(11) NOT NULL auto_increment,
  `subject` varchar(200) NOT NULL,
  `body` varchar(2000) NOT NULL,
  `total` int(10) NOT NULL,
  `traffic` int(10) NOT NULL COMMENT 'gigabytes',
  `username` varchar(200) NOT NULL,
  `status` enum('UNPAID','PAID','CHARGED','REVOKED') default NULL,
  `type` enum('VPN') default NULL,
  `alipay_trade_no` varchar(2000) default NULL,
  `alipay_buyer_email` varchar(2000) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `ticket_threads`;
CREATE TABLE `ticket_threads` (
  `id` int(11) NOT NULL auto_increment,
  `title` varchar(200) NOT NULL,
  `username` varchar(200) NOT NULL,
  `email` varchar(200) NOT NULL,
  `category` varchar(200) NOT NULL,
  `status` enum('open','closed') NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `tickets`;
CREATE TABLE `tickets` (
  `id` int(11) NOT NULL auto_increment,
  `thread_id` int(11) NOT NULL,
  `role` enum('client','pardiff') NOT NULL,
  `ts` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `username` varchar(200) default NULL,
  `content` text NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `traffic_logs`;
CREATE TABLE `traffic_logs` (
  `id` int(11) NOT NULL auto_increment,
  `ts` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `traffic` bigint(20) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `transactions_records`;
CREATE TABLE `transactions_records` (
  `id` int(11) NOT NULL auto_increment,
  `credit` bigint(20) default NULL,
  `operator` varchar(200) default NULL,
  `username` varchar(200) NOT NULL,
  `ts` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `note` varchar(2000) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB;
