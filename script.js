// function submitWeight() {
//     var weight = document.getElementById('weight').value;
//     var xhr = new XMLHttpRequest();
//     xhr.open('POST', '/setWeight', true);
//     xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
//     xhr.onreadystatechange = function() {
//       if (xhr.readyState == 4 && xhr.status == 200) {
//         document.getElementById('result').innerText = xhr.responseText;
//       }
//     };
//     xhr.send('weight=' + weight);
//   }

//   function submitWeight() {
//     var weight = document.getElementById('weight').value;
//     var xhr = new XMLHttpRequest();
//     xhr.open('POST', '/setWeight', true);
//     xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
//     xhr.onreadystatechange = function() {
//       if (xhr.readyState == 4 && xhr.status == 200) {
//         document.getElementById('result').innerText = xhr.responseText;
//         // After setting weight, fetch average acceleration
//         fetchAverageAccel();
//       }
//     };
//     xhr.send('weight=' + weight);
//   }

//   function fetchAverageAccel() {
//     var xhr = new XMLHttpRequest();
//     xhr.open('GET', '/averageAccel', true);
//     xhr.onreadystatechange = function() {
//       if (xhr.readyState == 4 && xhr.status == 200) {
//         var averageAccel = parseFloat(xhr.responseText);
//         var weight = parseFloat(document.getElementById('currentWeight').innerText);
//         var punchingPower = averageAccel * (weight / 100) * 50;
//         document.getElementById('averageAccel').innerText = 'Average Acceleration: ' + averageAccel.toFixed(2) + ', Punching Power: ' + punchingPower.toFixed(2) + ' N';
//       }
//     };
//     xhr.send();
//   }
  

  
//   // Fetch initial average acceleration and set up interval
//   fetchAverageAccel();
//   setInterval(fetchAverageAccel, 1000);

