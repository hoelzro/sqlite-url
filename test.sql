.load ./url
.selftest --init

insert into selftest (op, cmd, ans) values
    ('run', "select url_scheme('http://example.com')", 'http'),
    ('run', "select url_scheme('https://example.com')", 'https');

.selftest
