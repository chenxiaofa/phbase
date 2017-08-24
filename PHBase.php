<?php
/**
 * Created by PhpStorm.
 * User: xfachen
 * Date: 2017/8/21
 * Time: 10:58
 */
class PHBase{
    public function __construct($host='127.0.0.1', $port='9090')
    {
    }

    /**
     * @throws PHBaseException
     * @return bool
     */
    public function connect(){}

    /**
     * @param string $table 表名
     * @param string $rowKey Key
     * @param null $family 列族
     * @param null $qualifier 字段名
     * @return array|null
     */
    public function get($table, $rowKey, $family=NULL, $qualifier=NULL){}

    /**
     * @param string $table 表名
     * @param string $rowKey Key
     * @param null $family 列族
     * @param null $qualifier 字段名
     * @return boolean
     */
    public function exists($table, $rowKey, $family=NULL, $qualifier=NULL){}
}

class PHBaseException extends \Exception{};