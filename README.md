# 请注意，这个Pardiff VPN是很多年前我不是很懂web的时候做的，我觉得不是很适合作为生产环境使用，小心谨慎


# ParDiff VPN source code

Features of ParDiff VPN includes,
 * Generic password for OpenVPN, PPTP, Web Login
 * Charge users by traffic
 * Detailed recording system
 * Price/Usage separated
 * Usage can be sold as voucher/SN, xtremely easy to manage
 * Graphic report system
 * Massive mail-out system
 * Support system (ticket system)

It is consisted of the following several components,
 * static web site
 * database schema - store all billing information
 * cronjobs - generate usage accounting, aggregation, reporting, monitoring traffic
 * init scripts - mainly security settings, protect system from hacker attack or inappropriate user activities
 * cgi interfaces
  - admin interface
  - user interface
 * interface with AliPay API (not released yet)
 * scripts to generate/remove/manage credit points

To be functioning, administrators are required to install the following packages. They are packaged separately to avoid license issue. They are *NOT* part of this software.
 * modified openvpn and pptp plugins

