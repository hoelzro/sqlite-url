.load ./url
.selftest --init

insert into selftest (op, cmd, ans) values
    ('memo', 'url_user tests', ''),
    ('run', "select url_user('http://example.com')", ''),
    ('memo', 'url_scheme tests', ''),
    ('run', "select url_scheme('http://example.com')", 'http'),
    ('run', "select url_scheme('https://example.com')", 'https');

.selftest
