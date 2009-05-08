<?php

    $x = $_REQUEST['lastX'];
    $y = $_REQUEST['lastY'];

    $r = array($x+1 , round(rand(0,100)));

    echo json_encode($r);

?>
