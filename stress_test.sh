PrintLog()   {
	echo "$(date '+%Y-%m-%d %H:%M:%S:%N') $1"
}

if [ $# -ne 2 ]; then
	PrintLog "Usage: ./stress_test.sh [sessions] [requests]"
	exit 1
fi

if [[ ! $1 =~ ^[0-9]+$ ]]; then
	PrintLog "The first parameter must be a number"
	exit 1
fi

if [[ ! $2 =~ ^[0-9]+$ ]]; then
	PrintLog "The second parameter must be a number"
	exit 1
fi

declare -a cookies

SESSIONS=$1
REQUEST_PER_SESSION=$2
URL="http://127.0.0.1:8080"

PrintLog "Inicia obtencion de cookies de sesion"
for ((i=0; i<$SESSIONS; i++)); do
	COOKIE=$(curl -s -X POST -i -d "username=Alex_$i" $URL/login/vendor 2>/dev/null | grep session_id | awk -F' ' '{print $2}')
	if [ $? -ne 0 ]; then
		PrintLog "Ocurrio un error al obtener la sesion $i [$?]"
		exit 3
	fi

	cookies+=("${COOKIE%?}")
done
PrintLog "Se generaron $SESSIONS cookies"

PrintLog "Se obtiene el inventario de productos"
mapfile -t INVENTORY < <(curl -s -X GET -b "${cookies[0]}" $URL/inventory/view 2>/dev/null | grep -o 'PROD-[^<]*')
if [ $? -ne 0 ]; then
	PrintLog "Ocurrio un error al obtener el inventario con la sesion \"${cookies[0]}\" [$?]"
	exit 4
fi
PrintLog "Se obtuvieron ${#INVENTORY[@]} productos"

purchase() {
	SESSION=$1
	PrintLog "[$SESSION] Se agrega al carrito $(( $REQUEST_PER_SESSION * ${#INVENTORY[@]} )) productos"
	for ((r=0; r<$REQUEST_PER_SESSION; r++));do
		for i in "${INVENTORY[@]}"; do
			curl -s -X POST -b "$SESSION" -d "product_id=$i&quantity=1" $URL/cart/add 1>/dev/null 2>/dev/null
		done
	done

	PrintLog "[$SESSION] Se realiza checkout"

	local ERRORS+=$(curl -s -X POST -b "$SESSION" -d "payment_method=E" $URL/cart/checkout | grep -ci "err")

	if [ "$ERRORS" -ne 0 ]; then
		PrintLog "[$SESSION] Ocurrio un error al realizar la compra"
		exit 5
	fi
}

PrintLog "Inicia compra de $REQUEST_PER_SESSION productos por sesion"
for i in "${cookies[@]}"; do
	purchase "$i" &
done
wait
PrintLog "Finalizo compra de productos"

