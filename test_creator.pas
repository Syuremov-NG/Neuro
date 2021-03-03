uses GraphABC;
var
p: Picture;
t:text;
c:char;
begin
p:= Picture.Create('image.png'); //Открытие файла
p.Draw(250,250, 128, 128);
writeln('Какой символ на картинке: ');
read(c);


assign(t, 'test.txt');//Создание файла test
t.Rewrite();//Будет происходить ПЕРЕЗАПИСЬ
//t.Append(); //ДОЗАПИСЬ

for i:integer:= 0 to p.Height-1 do//Записываем матрицу
begin
  for j:integer:= 0 to p.Width-1 do
    write(t, (1.0 - (p.GetPixel(j,i).R / 255)):0:3, ' ');
  writeln(t, '');//Новая строка матрицы
end;
writeln(t, c);//Правильный ответ
t.Close();//Закрытие файла


writeln('done!');
window.Close();
end.