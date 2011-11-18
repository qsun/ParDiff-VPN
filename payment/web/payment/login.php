<?php

require_once('db-config.php');
if (!$mysqli) {
    echo("Failed to connect database");
    exit(0);
}

// session_start();

// get the password ref
$password = $_POST['password'];
$username = $_POST['username'];

if ($stmt = $mysqli->prepare("SELECT * FROM login WHERE username = ? AND password = ?")) {
    $stmt->bind_param("ss", $username, $password);
    $stmt->execute();
    if ($stmt->fetch()) {
        session_start();
        $_SESSION['username'] = $username;
        $_SESSION['password'] = sha1($username + $password + $username);
        header("Location: {$config[view_page]}");
        exit(0);
    }
}

// check password ref
// check the trasaction
// if there is, show to user

header("Location: {$config[login_page]}");
?>

