<?php

ini_set("memory_limit","256M");

$img_base64 = $_POST["COLO_DAT"];
$img_base64 = str_replace(' ', '+', $img_base64);
// base64をデコードする
$img_base64 = base64_decode($img_base64);
// base64から画像リソース化
$image = imagecreatefromstring($img_base64);

$file_name = "tmp.jpeg";

if ($image !== false)
{
    header('Content-Type: image/jpeg');
	imagejpeg($image, $file_name);
	imagedestroy($image);

	chmod($file_name, 0777);

	$apikey = '0EpMVBPQRftwi57SIvJ0lxVAdmHBCc7afSF_m-0ENMqN';

	$headers = array(
        'Content-Type:application/json',
        'Authorization: Basic '. base64_encode("apikey:$apikey"), 
    );
	$options = array('http' => array(
        header => implode("\r\n", $headers )
    ));

	$url = "https://gateway.watsonplatform.net/visual-recognition/api/v3/classify?";
	$url .= "&version=2018-03-19";
	$url .= "&Accept-Language=ja";
	$url .= "&url=http://yusk1450.sakura.ne.jp/watson_php/tmp.jpeg";

	$res = file_get_contents($url, false, stream_context_create($options));
	$json = json_decode($res, true);

	$cur_keywords = array();
	foreach($json["images"]["0"]["classifiers"]["0"]["classes"] as $key => $val)
	{
		// echo $val["class"];
		$cur_keywords []= $val["class"];
	}
	echo json_encode($cur_keywords[0]);
}
else
{
    echo "[\"An error occurred\"]";
}

?>