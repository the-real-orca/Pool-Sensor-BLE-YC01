R"(
<!DOCTYPE HTML><html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <body>
  <p>reload previouse page</p>
  <script>
    if (document.referrer) {
      window.location.href = document.referrer + (document.referrer.includes('?') ? '&' : '?') + 'reload=' + new Date().getTime();
    } else {
      window.location.href = '/';
    }
  </script>
</body>
</html>
)"