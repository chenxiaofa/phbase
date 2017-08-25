<?php
/**
 * Created by PhpStorm.
 * User: xfachen
 * Date: 2017/8/21
 * Time: 10:59
 */





$pidArray = [];
$files = glob('x???');
$s = microtime(1);
$pid = 0;
while($files) {
    while (count($pidArray) >= 10) {
        foreach($pidArray as $pid) {
            if (pcntl_waitpid($pid, $status, WNOHANG) != 0) {

                unset($pidArray[$pid]);
            }
        }
    }
    $f = array_pop($files);
    $pid = pcntl_fork();
    if ($pid == 0)break;
    $pidArray[$pid] = $pid;
}

if ($pid > 0) {
    foreach($pidArray as $pid){
        pcntl_waitpid($pid, $status);
    }
    echo "\n\n","total used:",(microtime(1)-$s)*1000,"\n\n";
    exit;
}


$client = new PHBase('192.168.234.236', 9090);
$client->connect();

$total = 0;
$count = 0;
foreach (file($f) as $udid){
    $udid = trim($udid);
    $s = microtime(1);
    $result = $client->get('testhbase', $udid.'_ef67aef2be557d56d80ac71c8e7fbb04', 'info');
    $total += microtime(1)-$s;
    $count++;
}

echo "use:",($total*1000)," count:",$count,"\n";

