<?php

$config = parse_ini_file('config.ini');

if (is_array($config)) {
    $mysqli = new mysqli($config[host], 
                         $config[username],
                         $config[password],
                         $config[db]);
}

function get_user($username)
{
    global $mysqli;
    if ($stmt = $mysqli->prepare("SELECT password FROM login WHERE username = ?")) {
        $stmt->bind_param("s", $username);
        $stmt->execute();
        $stmt->bind_result($password);
        
        $ret_ref = $stmt->fetch();
        if ($ret_ref) {
            return array('username' => $username, 
                         'password' => $password);
        }
    }
    
    return false;
}

function require_login() 
{
    global $config;
    global $mysqli;
    global $current_user;
    
    session_start();
    
    $given_password = $_SESSION['password'];
    $given_username = $_SESSION['username'];
    
    $user = get_user($given_username);
    if ($user && $given_password == sha1($given_username + $user['password'] + $given_username)) {

        $current_user = $given_username;
        return $current_user;
    }

    header("Location: {$config[login_page]}");
    exit(0);
}


?>