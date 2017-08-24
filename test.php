<?php
/**
 * Created by PhpStorm.
 * User: xfachen
 * Date: 2017/8/21
 * Time: 10:59
 */

$a = new PHBase('192.168.234.236', 9090);
$a->connect();
try{
    $exists = $a->exists('user_base', '17296');
    debug_zval_dump($exists);

    $result = $a->get('user_base', '17296', 'info', 'age');
    debug_zval_dump($result);

}catch (\PHBaseException $e)
{
    print_r($e);
}


