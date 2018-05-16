<?php
echo('<html lang="en">');
echo('<head>');
echo('    <meta charset="UTF-8">');
echo('    <title>progonka</title>');
echo('</head>');
echo('<body style="text-align: center">');
if(isset($_POST['submit1'])){
    $size = $_POST['dimension'];
    echo('<TABLE style="text-align: center; margin: auto">');
    echo('<form action = "progonka.php" method="post">');
    echo('<TR>');
    echo('<TD>');
        echo('<TABLE border="1">');
        for($i=1; $i<=$size; $i++){
            echo('<TR>');
            for($j=1; $j<=$size; $j++) {
                echo('<TD>');
                if ($i==$j || $i+1==$j || $j+1==$i) {
                    echo "<input type='text' value='1' size='4' name='a" . ($j + (($i - 1) * $size)) . "'>";
                } else {
                    echo "<input type='text' value='0' size='4' name='a" . ($j + (($i - 1) * $size)) . "'>";
                }
                echo('</TD>');
            }
            echo('</TR>');
        }
        echo('</TABLE>');
    echo('</TD>');

    echo('<TD>x</TD>');

    echo('<TD>');
        echo('<TABLE border="1">');
        for($i=0; $i<$size; $i++) {
            echo "<TR><TD><B>x<SUB>".$i."</SUB></B></TD></TR>";
        }
        echo('</TABLE>');
    echo('</TD>');

    echo('<TD>=</TD>');

    echo('<TD>');
        echo('<TABLE border="1">');
        for($i=1; $i<=$size; $i++) {
            echo "<TR><TD><input type = 'text' size='4' align='right' value='0' name='b".$i."'</TD></TR>";
        }
        echo('</TABLE>');
    echo('</TD>');
    echo('</TR>');

    echo('<TR>');
    echo('<TD>');
    echo('<p><input type = "submit" title="submit2" name="submit2"></p>');
    echo"<p><input type = 'hidden' title='dimension' name='dimension' value='".$size."'></p>";
    echo('</TD>');
    echo('</TR>');
    echo('</TABLE>');

} elseif ( isset($_POST['submit2'])) {
    $size = $_POST['dimension'];
    $gamma = [];
    $alpha = [];
    $beta = [];
    $gamma[1] = $_POST['a1'];
    $alpha[1] = (-$_POST['a2'])/$gamma[1];
    $beta[1] = $_POST['b1']/$gamma[1];
    for($i=2; $i<$size;$i++){
        $diag = ($i + (($i - 1) * $size));
        $b = $_POST['a'.$diag];
        $a = $_POST['a'.($diag - 1)];
        $c = $_POST['a'.($diag + 1)];
        $d = $_POST['b'.$i];
        $gamma[$i] = $b + $a*$alpha[$i-1];
        $alpha[$i] = -$c/$gamma[$i];
        $beta[$i] = ($d - $a*$beta[$i-1])/$gamma[$i];
    }

    $b = $_POST['a'.($size*$size)];
    $a = $_POST['a'.($size*$size - 1)];
    $d = $_POST['b'.$size];

    $gamma[$size] = $b + $a*$alpha[$size-1];
    $beta[$size] = ($d - $a*$beta[$size-1])/$gamma[$size];

    $x = [];
    $x[$size] = $beta[$size];

    for($i = $size-1; $i > 0; $i--) {
        $x[$i] = $alpha[$i] * $x[$i+1] + $beta[$i];
    }

    echo('<p><b>Результаты расчета:</b></p>');
    for($i = 1; $i <=$size; $i++) {
        echo "<p>x".($i-1)." = ".$x[$i]."</p>";
    }

} else {

echo('    <form action="progonka.php" method="post">');
echo('        <b>Введите размерность матрицы:</b>');
echo('        <input type="text" size=2 title="dimension" name="dimension">');
echo('        <p><input type = "submit" title="submit1" name="submit1"></p>');
echo('    </form>');
}
echo('</body>');
echo('</html>');