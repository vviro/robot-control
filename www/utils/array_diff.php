<?php
/**	
 * Recursive difference of two arrays
 *	
 * PHP version 5
 *
 * @category PHP
 * @package  Game
 * @author   Vladimir Viro <vviro@mail.ru>
 * @license  BSD License
 * @link     http://seoka.de/game
 */

/**	
 * Recursive difference of two arrays
 *	
 * @param array $a1 Array, from which $a2 is subtacted
 * @param array $a2 Array, wich is subtracted from $a1
 *
 * @return array $diff Array, containing elements from $a2, that are not contained in $a1
 *
 */
function arrDiff($a1, $a2) 
{
    $diff = array();
    foreach ($a1 as $k=>$v) {
        unset($dv);
        if (is_int($k)) {
            // Compare values
            if (in_array($v, $a2)===false and !is_array($v)) $dv=$v;
            else if (in_array($v, $a2)===false and is_array($v)) $dv=arrDiff($v, $a2[$k]);
            else if (($v === 0 and $a2[$k] !== 0) or ($v === '0' and $a2[$k] !== '0')) $dv=$v;
            if(isset($dv) and $dv != array()) $diff[]=$dv;
        } else {
            // Compare noninteger keys
            if ((!$a2[$k] and $a2[$k] != 0) or ($v != 0 and $a2[$k] == 0)) $dv=$v;
            else if (is_array($v)) $dv=arrDiff($v, $a2[$k]);
            else if ($a2[$k] != $v) $dv=$v;
            if (isset($dv) and $dv != array()) $diff[$k]=$dv;
        }   
    }
    return $diff;
}
?>
