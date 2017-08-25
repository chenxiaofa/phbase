<?php
/**
 * Created by PhpStorm.
 * User: xfachen
 * Date: 2017/8/25
 * Time: 12:07
 */

$client = new PHBase('192.168.234.236', 9091);
$client->connect();

$result = $client->get('user_base', '10271', 'info');

print_r($result);