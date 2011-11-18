drop table payment_transactions;

create table payment_transactions (
id int not null auto_increment,
subject varchar(200) not null,
body varchar(2000) not null,
total int(10) not null,
traffic int(10) not null COMMENT 'gigabytes',
username varchar(200) not null,
status ENUM ('UNPAID', 'PAID', 'CHARGED', 'REVOKED'),
`type` ENUM('VPN'),
alipay_trarde_no VARCHAR(2000) default NULL,
alipay_buyer_email VARCHAR(2000) default NULL,
primary key (id)) ENGINE=InnoDB CHARSET=utf8;


insert into payment_transactions (subject, body, total, traffic, username, status, `type`) values ('从前有座山', '山上有座庙', 1, 10, 'test', 'UNPAID', 'VPN');
