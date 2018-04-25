<?php
/**
 * Created by PhpStorm.
 * User: nostromo
 * Date: 5/21/17
 * Time: 7:44 PM
 */

if(!isset($_POST["mode"])) {
    echo '<form method="POST" action="/Calculations.php">';
    echo '<input type="hidden" name="mode" value="1">';
    echo '<input type="hidden" name="dims" value="' . $_POST["dims"] . '">';
    echo "<table>";
    echo "<tr>";
    echo "<td>";
        echo "<table>";
        for ($y = 0; $y < $_POST["dims"]; $y++) {
            echo "<tr>";
            for ($x = 0; $x < $_POST["dims"]; $x++) {
                echo "<td>";
                    echo '<input type="text" name="' . $x . '_' . $y . '" value="0">';
                echo "</td>";
            }
        }
        echo "</table>";
    echo "</td>";
    echo "<td align='center'>";
        echo "x";
    echo "</td>";
    echo "<td>";
        echo "<table border='1'>";
        for($i = 0; $i < $_POST["dims"]; $i++){
            echo "<tr>";
            echo "<td>";
                echo "X" . ($i+1);
            echo "</td>";
            echo "</tr>";
        }
        echo "</table>";
    echo "</td>";

    echo "<td align='center'>";
        echo "=";
    echo "</td>";

    echo "<td>";
    echo "<table>";
    for($i = 0; $i < $_POST["dims"]; $i++){
        echo "<tr>";
        echo "<td>";
        echo "<input type='number' name='" . $i . "' value='0'>";
        echo "</td>";
        echo "</tr>";
    }
    echo "</table>";
    echo "</td>";

    echo "</tr>";
    echo "</table>";
    echo "<button type='submit'>Далее</button>";
    echo '</form>';
} else {
    $a = array();   // Диагональ под главной
    $c = array();   // Главная диагональ
    $b = array();   // Диагональ над главной
    $f = array();   // Парвая часть
    $a[0] = 0;
    $b[$_POST["dims"]-1] = 0;
    for($i = 0; $i < $_POST["dims"]; $i++){
        if($i > 0) {
            $index = '' . ($i - 1) . '_' . $i;
            $a[$i] = $_POST[$index];
        }
        $index = '' . $i . '_' . $i;
        $c[$i] = $_POST[$index];
        //echo $c[$i] . "<br>";
        if($i < $_POST["dims"]-1) {
            $index = '' . ($i + 1) . '_' . $i;
            $b[$i] = $_POST[$index];
        }
        $f[$i] = $_POST[$i];
    }
    //print_r($c);
    $ca = array();
    $fa = array();
    $ca[0] = $c[0];
    $fa[0] = $f[0];
    for($i = 1; $i < $_POST["dims"]; $i++){
        //echo "<br>C: ";
        //print_r($c);
        //echo "<br>CA: ";
        //print_r($ca);
        $ca[$i] = $c[$i] - ($a[$i]*$b[$i-1])/$ca[$i-1];
        $fa[$i] = $f[$i] - ($a[$i]*$fa[$i-1])/$ca[$i-1];
    }

    $x = array();

    $x[$_POST["dims"]-1] = $fa[$_POST["dims"]-1]/$ca[$_POST["dims"]-1];

    for ($i = $_POST["dims"] - 2; $i >= 0; $i--) {
        $x[$i] = ($fa[$i] - $b[$i] * $x[$i + 1]) / $ca[$i];
    }

    for($i = 0; $i < $_POST["dims"]; $i++){
        echo "   X" . ($i+1) . " = " . $x[$i];
        echo "<br>";
    }
}