<?php
/*
	ospfd.inc
	Copyright (C) 2010 Ermal Lu�i
	part of pfSense
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

function ospfd_install_conf() {
	global $config, $g;
	
	conf_mount_rw();
	
	if ($config['installedpackages']['ospfd']['rawconfig'] && $config['installedpackages']['ospfd']['rawconfig']['item']) {
		// if there is a raw config specifyed in tthe config.xml use that instead of the assisted config
		$conffile = implode("\n",$config['installedpackages']['ospfd']['rawconfig']['item']);
		//$conffile = $config['installedpackages']['ospfd']['rawconfig'];
	} else {
		// generate ospfd.conf based on the assistant
		if($config['installedpackages']['ospfd']['config']) 
			$ospfd_conf = &$config['installedpackages']['ospfd']['config'][0];
		else {
			log_error("OSPFd: No config data found.");
			return;
		}
		if($config['installedpackages']['ospfd']['config'][0]['row'])
			$ospfd_rows = &$config['installedpackages']['ospfd']['config'][0]['row'];	
    
		$conffile = "# This file was created by the pfSense package manager.  Do not edit!\n\n";
    
		// Specify router id
		if($ospfd_conf['routerid']) 
			$conffile .= "router-id {$ospfd_conf['routerid']}\n";

		if ($ospfd_conf['updatefib'])
			$conffile .= "fib-update no\n";

		if ($ospfd_conf['Redistributeconnectedsubnets'])
			$conffile .= "redistribute connected\n";

		if ($ospfd_conf['Redistributedefaultroute'])
			$conffile .= "redistribute default\n";

		if ($ospfd_conf['Resdistributestatic'])
			$conffile .= "redistribute static\n";
		
		$conffile .= "area {$ospfd_conf['Area']} {\n";


		$conffile .= "}\n";
	}	

	$fd = fopen("/usr/local/etc/ospfd.conf", "w");
	
	// Write out the configuration file
	fwrite($fd, $conffile);
	
	// Close file handle
	fclose($fd);
	
	// Create rc.d file
	$fd = fopen("/usr/local/etc/rc.d/ospfd.sh","w");
	fwrite($fd, "#!/bin/sh\n\n");
	fwrite($fd, "# This file was created by the pfSense package manager.  Do not edit!\n\n");	
	fwrite($fd, "/usr/local/sbin/ospfd -f /usr/local/etc/ospfd.conf\n");
	fclose($fd);
	exec("chmod a+rx /usr/local/etc/rc.d/ospfd.sh");
	exec("chmod a-rw /usr/local/etc/ospfd.conf");
	exec("chmod u+rw /usr/local/etc/ospfd.conf");
	
	// ospfd process running?  if so reload, elsewise start.
	if(is_ospfd_running() == true) {
		exec("/usr/local/sbin/ospfctl reload");
	} else {
		exec("/usr/local/sbin/ospfd");
	}
	
	conf_mount_ro();
}

// get the raw ospfd confi file for manual inspection/editing
function ospfd_get_raw_config() {
  return file_get_contents("/usr/local/etc/ospfd.conf");
}

// serialize the raw ospfd confi file to config.xml
function ospfd_put_raw_config($conffile) {
  global $config;
  if ($conffile == "")
    unset($config['installedpackages']['ospfd']['rawconfig']);
  else {
    $config['installedpackages']['ospfd']['rawconfig'] = array();
    $config['installedpackages']['ospfd']['rawconfig']['item'] = explode("\n",$_POST['ospfd_raw']);
    //$config['installedpackages']['ospfd']['rawconfig'] = $conffile;
  }
}

function is_ospfd_running() {
	$status = `ps awux | grep ospfd | grep "parent" | grep -v grep | wc -l | awk '{ print \$1 }'`;
	if(intval($status) > 0) 
		return true;
	else 
		return false;
}

?>